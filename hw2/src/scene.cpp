#include "scene.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>

unsigned int get_command(const std::string& command) {
    if (command == "")                      return COMMAND_EMPTY;
    if (command == "DIMENSIONS")            return COMMAND_DIMENSIONS;
    if (command == "BG_COLOR")              return COMMAND_BG_COLOR;
    if (command == "CAMERA_POSITION")       return COMMAND_CAMERA_POSITION;
    if (command == "CAMERA_RIGHT")          return COMMAND_CAMERA_RIGHT;
    if (command == "CAMERA_UP")             return COMMAND_CAMERA_UP;
    if (command == "CAMERA_FORWARD")        return COMMAND_CAMERA_FORWARD;
    if (command == "CAMERA_FOV_X")          return COMMAND_CAMERA_FOV_X;
    if (command == "NEW_PRIMITIVE")         return COMMAND_NEW_PRIMITIVE;
    if (command == "PLANE")                 return COMMAND_PLANE;
    if (command == "ELLIPSOID")             return COMMAND_ELLIPSOID;
    if (command == "BOX")                   return COMMAND_BOX;
    if (command == "POSITION")              return COMMAND_POSITION;
    if (command == "ROTATION")              return COMMAND_ROTATION;
    if (command == "COLOR")                 return COMMAND_COLOR;
    if (command == "RAY_DEPTH")             return COMMAND_RAY_DEPTH;
    if (command == "AMBIENT_LIGHT")         return COMMAND_AMBIENT_LIGHT;
    if (command == "NEW_LIGHT")             return COMMAND_NEW_LIGHT;
    if (command == "LIGHT_INTENSITY")       return COMMAND_LIGHT_INTENSITY;
    if (command == "LIGHT_DIRECTION")       return COMMAND_LIGHT_DIRECTION;
    if (command == "LIGHT_POSITION")        return COMMAND_LIGHT_POSITION;
    if (command == "LIGHT_ATTENUATION")     return COMMAND_LIGHT_ATTENUATION;
    if (command == "METALLIC")              return COMMAND_METALLIC;
    if (command == "DIELECTRIC")            return COMMAND_DIELECTRIC;
    if (command == "IOR")                   return COMMAND_IOR;

    return -1;
}

Camera::Camera(float fov_x) : fov_x(fov_x) {}

///////////////////
// SCENE LOADING //
///////////////////

std::pair<std::unique_ptr<Primitive>, std::string> LoadPrimitive(std::istream &in) {
    std::unique_ptr<Primitive> primitive;

    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmd_name;
        ss >> cmd_name;

        auto cmd = get_command(cmd_name);
        if(cmd==COMMAND_EMPTY) break;

        switch (cmd) {
            case COMMAND_ELLIPSOID: {
                Point r;
                ss >> r;
                primitive = std::unique_ptr<Primitive>(new Ellipsoid(r));
                break;
            }
            case COMMAND_PLANE: {
                Point n;
                ss >> n;
                primitive = std::unique_ptr<Primitive>(new Plane(n));
                break;
            }
            case COMMAND_BOX: {
                Point s;
                ss >> s;
                primitive = std::unique_ptr<Primitive>(new Box(s));
                break;
            }
            case COMMAND_COLOR: {
                ss >> primitive->col;
                break;
            }
            case COMMAND_POSITION: {
                ss >> primitive->pos;
                break;
            }
            case COMMAND_ROTATION: {
                ss >> primitive->rotator;
                primitive->rotator.w *= -1; // ?! ?! ?! ? ! ?! ?!? !?!? !?? !?!?! ?!?!?!!?
                break;
            }            
            case COMMAND_METALLIC: {
                primitive->material = Material::METALLIC;
                break;
            }            
            case COMMAND_DIELECTRIC: {
                primitive->material = Material::DIELECTRIC;
                break;
            }            
            case COMMAND_IOR: {
                ss >> primitive->ior;
                break;
            }            
            
            default: {
                std::cerr << "unexpected light(" << cmd_name << ")" << std::endl;
                return std::make_pair(std::move(primitive), cmd_name);
            }
        }
    }
 
    return std::make_pair(std::move(primitive), "");
}

std::unique_ptr<Light> createLightPtr(const Color& intens, const Point& pos, const Color& att, const Point& dir, bool isDirectedLight) {
    if (isDirectedLight) {
        return std::move(std::make_unique<DirectedLight>(DirectedLight(intens, dir)));
    } else {
        return std::move(std::make_unique<DotLight>(DotLight(intens, pos, att)));
    }
}

std::pair<std::unique_ptr<Light>, std::string> LoadLight(std::istream &in) {
    Color intens;
    Point pos;
    Color att;
    Point dir;

    bool isDirectedLight = false;

    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmd_name;
        ss >> cmd_name;

        auto cmd = get_command(cmd_name);
        if(cmd==COMMAND_EMPTY) break;

        switch (cmd) {
            case COMMAND_LIGHT_INTENSITY: {
                ss >> intens;
                break;
            }
            case COMMAND_LIGHT_POSITION: {
                ss >> pos;
                break;
            }
            case COMMAND_LIGHT_DIRECTION: {
                ss >> dir;
                isDirectedLight = true;
                break;
            }
            case COMMAND_LIGHT_ATTENUATION: {
                ss >> att;
                break;
            }
            
            default: {
                std::cerr << "unexpected primitive(" << cmd_name << ")" << std::endl;
                std::unique_ptr<Light> light(createLightPtr(intens, pos, att, dir, isDirectedLight));
                return std::make_pair(std::move(light), cmd_name);
            }
        }
    }
    
    std::unique_ptr<Light> light(createLightPtr(intens, pos, att, dir, isDirectedLight));
 
    return std::make_pair(std::move(light), "");
}

void Scene::Load(std::istream &in) {
    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmd_name;
        ss >> cmd_name;

    pasrse_command_again:
        auto cmd = get_command(cmd_name);
        if(cmd==COMMAND_EMPTY) continue;

        switch (cmd) {
            case COMMAND_DIMENSIONS: {
                ss >> cam.width >> cam.height;
                break;
            }
            case COMMAND_BG_COLOR: {
                ss >> background;
                break;
            }
            case COMMAND_CAMERA_POSITION: {
                ss >> cam.pos;
                break;
            }
            case COMMAND_CAMERA_RIGHT: {
                ss >> cam.right;
                break;
            }
            case COMMAND_CAMERA_UP: {
                ss >> cam.up;
                break;
            }
            case COMMAND_CAMERA_FORWARD: { 
                ss >> cam.forward;
                break;
            }
            case COMMAND_CAMERA_FOV_X: {
                ss >> cam.fov_x;
                break;
            }
            case COMMAND_NEW_PRIMITIVE: {
                auto [primitive, rest_cmd] = LoadPrimitive(in);
                primitives.push_back(std::move(primitive));
                cmd_name = rest_cmd;
                if(!cmd_name.empty()) {
                    goto pasrse_command_again;
                }
                break;
            }
            case COMMAND_RAY_DEPTH: {
                ss >> ray_depth;
                break;
            }
            case COMMAND_AMBIENT_LIGHT: {
                ss >> ambient_light;
                break;
            }
            case COMMAND_NEW_LIGHT: {
                auto [light, rest_cmd] = LoadLight(in);
                lights.push_back(std::move(light));

                cmd_name = rest_cmd;
                if(!cmd_name.empty()) {
                    goto pasrse_command_again;
                }
                break;
            }
            default: {
                std::cerr << "unexpected command(" << cmd_name << ")" << std::endl;
                break;
            }
        }
    }
}

/////////////////////
// SCENE RENDERING //
/////////////////////

Ray Camera::GetToRay(int x, int y) const {
    float tan_fov_x = tan(fov_x / 2);
    float tan_fov_y = tan_fov_x * height / width;

    float nx =        (2 * (x + 0.5) / width  - 1) * tan_fov_x;
    float ny = -1.f * (2 * (y + 0.5) / height - 1) * tan_fov_y; // -1.f becase of reversed order in render

    return {pos, nx*right + ny*up + 1.f*forward};
}

ray_intersection_t Scene::RayIntersection(const Ray &ray, float tmax) const {
    ray_intersection_t ret;
    ret.id = -1;

    float closest_dist = -1;
    int cur_id = 0;
    for (auto &primitive : primitives) {
        auto intersection = primitive->colorIntersect(ray);
        if (intersection.has_value()) {
            auto [isec, _] = intersection.value();
            if (isec.t <= tmax) {
                if (closest_dist == -1 || isec.t < closest_dist) {
                    closest_dist = isec.t;
                    ret = {isec, cur_id};
                }
            }
        }
        ++cur_id;
    }
    return ret;
}

static Point GetReflection(const Point& normal, const Point& dir) {
    return dir - 2.0 * normal * glm::dot(normal, dir);
}

Color Scene::RayTrace(const Ray& ray, size_t ost_raydepth) const {
    if (ost_raydepth == 0) {
        return {0., 0., 0.};
    }

    auto raytrace = RayIntersection(ray, 1e18);
    if (raytrace.id == -1) {
        return background;
    }

    auto [t, normal, interior] = raytrace.isec;
    size_t id = raytrace.id;
    Point p = ray.o + t * ray.d;
    glm::vec3 reflect_dir = GetReflection(normal, glm::normalize(ray.d));

    assert(glm::length(normal) <= 1 + eps);
    
    switch (primitives[id]->material)
    {
    case Material::DIFFUSE: {
        Color summary_color = ambient_light;
        for (const auto &light : lights) {
            auto [color, dir, dist] = light->CalcLight(p);
            float koef = glm::dot(dir, normal);
            if(koef >= 0) { // otherwise light is behind
                bool is_point_between = (RayIntersection({p + eps * dir, dir}, dist).id != -1);
                if(!is_point_between) {
                    summary_color = {summary_color.rgb + koef * color.rgb}; 
                }
            }
        }
        summary_color = {summary_color.rgb * primitives[id]->col.rgb};
        return summary_color;
        break;
    }
    case Material::METALLIC: {   
        Color reflected_color = RayTrace({p + eps * reflect_dir, reflect_dir}, ost_raydepth-1);     
        return {primitives[id]->col.rgb * reflected_color.rgb};
        break;
    }
    case Material::DIELECTRIC: {
        Color reflected_color = RayTrace({p + eps * reflect_dir, reflect_dir}, ost_raydepth-1);
        Color summary_color = reflected_color;

        float eta1 = 1., eta2 = primitives[id]->ior;
        if (interior) {
            std::swap(eta1, eta2);
        }

        glm::vec3 dir = -1. * glm::normalize(ray.d);
        float dot_normal_dir = glm::dot(normal, dir);
        float sin_theta2 = eta1 / eta2 * sqrt(1 - dot_normal_dir * dot_normal_dir);
        if (fabs(sin_theta2) > 1.) {
            return summary_color;
        }

        float cos_theta2 = sqrt(1 - sin_theta2 * sin_theta2);
        glm::vec3 refracted_dir = eta1 / eta2 * (-1. * dir) + (eta1 / eta2 * dot_normal_dir - cos_theta2) * normal;
        Ray refracted = Ray(p + eps * refracted_dir, refracted_dir);
        Color refracted_component = RayTrace(refracted, ost_raydepth - 1);
        if (!interior) {
            refracted_component = refracted_component.rgb * primitives[id]->col.rgb;
        }

        float r0 = pow((eta1 - eta2) / (eta1 + eta2), 2.);
        float r = r0 + (1 - r0) * pow(1 - dot_normal_dir, 5.);
        return r * reflected_color.rgb + (1 - r) * refracted_component.rgb;
        break;
    }
    default:
        std::cerr<<"unknown material: primitive id(" << id << ")" << std::endl;
        break;
    }
    return {0.f, 0.f, 0.f};
}

void Scene::Render(std::ostream &out) const {
    out << "P6\n";
    out << cam.width << " " << cam.height << "\n";
    out << 255 << "\n";

    for (int y = 0; y < cam.height; ++y) {
        for (int x = 0; x < cam.width; ++x) {
            Color color = RayTrace(cam.GetToRay(x, y), ray_depth);
            color = AcesTonemap(color);
            color = GammaCorrected(color);

            uint8_t *rgb = color.toUInts();
            out.write(reinterpret_cast<char*>(rgb), 3);
        }
    }
}
