#include "scene.h"

#include <omp.h>

Camera::Camera(float fov_x) : fov_x(fov_x) {}

void Scene::InitScene() {
    InitBVH();
    InitDistribution();
}

/////////
// BVH //
/////////

void Scene::InitBVH() {
    uint32_t n = std::partition(primitives.begin(), primitives.end(), [](const Primitive &prim) {
        return prim.primitive_type != PRIMITIVE_TYPE::PLANE;
    }) - primitives.begin();
    scene_bvh = BVH_t(primitives, n);
}

///////////////////
// DISTRIBUTION  //
///////////////////

void Scene::InitDistribution() {
    std::vector<Distribution> prim_distribs;
    for (const Primitive& prim: primitives) {
        if (!(prim.emission.r() > 0 || prim.emission.g() > 0 || prim.emission.b() > 0)) {
            continue;
        }
        if (prim.primitive_type == PRIMITIVE_TYPE::BOX) {
            prim_distribs.emplace_back(std::move(Distribution(DISTRIB_TYPE::BOX, &prim)));
        } else if (prim.primitive_type == PRIMITIVE_TYPE::ELLIPSOID) {
            prim_distribs.emplace_back(std::move(Distribution(DISTRIB_TYPE::ELLIPSOID, &prim)));
        }
    }
    mix_distrib = Distribution(DISTRIB_TYPE::MIX, std::move(prim_distribs));
}

/////////////////////
// SCENE RENDERING //
/////////////////////

ray_intersection_t Scene::RayIntersection(const Ray &ray) const {
    ray_intersection_t ret;
    ret.id = -1;

    float closest_dist = INF;
    int cur_id = 0;
    for (auto &primitive : primitives) {
        if (primitive.primitive_type != PRIMITIVE_TYPE::PLANE) {
            ++cur_id;
            continue;
        }
        auto intersection = primitive.Intersect(ray);
        if (intersection.has_value()) {
            auto [t, _, __] = intersection.value();
            if (t < closest_dist) {
                closest_dist = t;
                ret = {intersection.value(), cur_id};
            }
        }
        ++cur_id;
    }

    ray_intersection_t ray_isec = scene_bvh.Intersect(primitives, ray, closest_dist);
    if (ray_isec.id != -1) {
        auto [t, _, __] = ray_isec.isec;
        if (t < closest_dist) {
            ret = ray_isec;
        }
    }
    
    return ret;
}

static Point GetReflection(const Point& normal, const Point& dir) {
    return dir - 2.0 * normal * glm::dot(normal, dir);
}

Color Scene::RayTrace(RANDOM_t& random, const Ray& ray, size_t ost_raydepth) {
    std::uniform_real_distribution<float>& uniform01 = random.uniform01.get();
    auto& rnd = random.rnd;

    if (ost_raydepth == 0) {
        return {0., 0., 0.};
    }

    auto raytrace = RayIntersection(ray);
    if (raytrace.id == -1) {
        return background;
    }

    auto [t, normal, interior] = raytrace.isec;
    size_t id = raytrace.id;
    Point p = ray.o + t * ray.d;
    
    Color other_color(0.f, 0.f, 0.f);
    switch (primitives[id].material)
    {
    case MATERIAL::DIFFUSE: {
        // L = E + 2*C*L_in(w)*dot(w,n)
        
        // moving from surface a lil bit
        glm::vec3 p_outer = p + eps * normal;
        // генерируем случайное направление при помощи mix_distribution
        glm::vec3 rand_dir = mix_distrib.Sample(random, p_outer, normal);

        // если не в той полусфере, то не учитываем дополнительный свет
        if (glm::dot(rand_dir,normal) <= 0) {
            break;
        }

        float pw = mix_distrib.Pdf(p_outer, normal, rand_dir);
        glm::vec3 L_in = RayTrace(random, Ray({p + eps * rand_dir, rand_dir}), ost_raydepth-1).rgb;
        glm::vec3 C = primitives[id].col.rgb;
        other_color = {(C / kPI) * L_in * glm::dot(rand_dir, normal) * (1 / pw)};
        break;
    }
    case MATERIAL::METALLIC: {   
        // L = E + C*L_in(R_n(w))

        glm::vec3 reflect_dir = GetReflection(normal, glm::normalize(ray.d));
        Color reflected_color = RayTrace(random, {p + eps * reflect_dir, reflect_dir}, ost_raydepth-1);     
        other_color = {primitives[id].col.rgb * reflected_color.rgb};
        break;
    }
    case MATERIAL::DIELECTRIC: {
        // sin(theta2) > 1 or coin flip < r => reflected
        // otherwise => refracted

        float eta1 = 1., eta2 = primitives[id].ior;
        if (interior) {
            std::swap(eta1, eta2);
        }

        glm::vec3 dir = -1. * glm::normalize(ray.d);
        float dot_normal_dir = glm::dot(normal, dir);
        float sin_theta2 = eta1 / eta2 * sqrt(1 - dot_normal_dir * dot_normal_dir);

        if (fabs(sin_theta2) > 1.) { // полное внутреннее отражение = вернуть отражённый
            glm::vec3 reflect_dir = GetReflection(normal, glm::normalize(ray.d));
            Color reflected_color = RayTrace(random, {p + eps * reflect_dir, reflect_dir}, ost_raydepth-1);
            other_color = reflected_color;
            break;
        }

        float r0 = pow((eta1 - eta2) / (eta1 + eta2), 2.);
        float r = r0 + (1 - r0) * pow(1 - dot_normal_dir, 5.);

        // с шансом r вернем отражённый / 1-r соответственно преломлённый
        if (uniform01(rnd) < r) {
            glm::vec3 reflect_dir = GetReflection(normal, glm::normalize(ray.d));
            Color reflected_color = RayTrace(random, {p + eps * reflect_dir, reflect_dir}, ost_raydepth-1);
            other_color = reflected_color;
            break;
        }

        float cos_theta2 = sqrt(1 - sin_theta2 * sin_theta2);
        glm::vec3 refracted_dir = eta1 / eta2 * (-1. * dir) + (eta1 / eta2 * dot_normal_dir - cos_theta2) * normal;
        Ray refracted = Ray(p + eps * refracted_dir, refracted_dir);
        Color refracted_color = RayTrace(random, refracted, ost_raydepth - 1);
        if (!interior) {
            refracted_color = { primitives[id].col.rgb * refracted_color.rgb };
        }
        other_color = refracted_color;
        break;
    }
    default:
        std::cerr<<"unknown material: primitive id(" << id << ")" << std::endl;
        break;
    }

    Color summary_color = {primitives[id].emission.rgb + other_color.rgb};
    return summary_color;
}

Ray Camera::GetToRay(float x, float y) const {
    float tan_fov_x = tan(fov_x / 2);
    float tan_fov_y = tan_fov_x * height / width;

    float nx =        (2 * x / width  - 1) * tan_fov_x;
    float ny = -1.f * (2 * y / height - 1) * tan_fov_y;

    return {pos, nx*right + ny*up + 1.f*forward};
}

Color Scene::Sample(RANDOM_t& random, unsigned int x, unsigned int y) {
    std::uniform_real_distribution<float>& uniform01 = random.uniform01.get();
    auto& rnd = random.rnd;

    Color summary(0.f, 0.f, 0.f);

    for(unsigned int i = 0; i < samples; ++i) {
        // сглаживаем
        float fx = x + uniform01(rnd);
        float fy = y + uniform01(rnd);
        summary = {summary.rgb + RayTrace(random, cam.GetToRay(fx, fy), ray_depth).rgb};
    }
    Color mean = {1.f / samples * summary.rgb };
    return mean;
}

void Scene::Render(std::ostream &out) {
    out << "P6\n";
    out << cam.width << " " << cam.height << "\n";
    out << 255 << "\n";

    std::vector<std::vector<glm::vec3>> pixels(cam.height, std::vector<glm::vec3>(cam.width));

    omp_set_num_threads(std::thread::hardware_concurrency());
    #pragma omp parallel for schedule(dynamic)
    for (unsigned int i = 0; i < cam.height * cam.width; i++) {
        std::minstd_rand rnd(i);
        std::uniform_real_distribution<float> uniform01{0.f, 1.f};
        std::normal_distribution<float> normal01{0.f, 1.f};
        RANDOM_t random{
            rnd, 
            uniform01, 
            normal01
        };
        unsigned int x = i % cam.width;
        unsigned int y = i / cam.width;
        Color color = Sample(random, x, y);
        color = AcesTonemap(color);
        color = GammaCorrected(color);

        pixels[y][x] = color.rgb;
    }

    for (unsigned int y = 0; y < cam.height; ++y) {
        for (unsigned int x = 0; x < cam.width; ++x) {
            Color color = pixels[y][x];

            uint8_t *rgb = color.toUInts();
            out.write(reinterpret_cast<char*>(rgb), 3);
            delete[] rgb;
        }
    }
}