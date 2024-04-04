#include "distributions.h"

///////////////////
//    UNIFORM    //
///////////////////

float Uniform01Distribution::sample(std::minstd_rand& rnd) {
    return uniform(rnd);
}

glm::vec3 Uniform01Distribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    auto& rnd = random.rnd;
    
    (void) x;
    (void) n;
    return glm::normalize(glm::vec3{uniform(rnd), uniform(rnd), uniform(rnd)});
}

float Uniform01Distribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) x;
    (void) n;
    (void) d;
    return 0.f;
}

///////////////////
//    NORMAL     //
///////////////////

float Normal01Distribution::sample(std::minstd_rand& rnd) {
    return normal(rnd);
}

glm::vec3 Normal01Distribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    auto& rnd = random.rnd;

    (void) x;
    (void) n;
    float flip1 = normal(rnd);
    float flip2 = normal(rnd);
    float flip3 = normal(rnd);
    return glm::normalize(glm::vec3{flip1, flip2, flip3});
}

float Normal01Distribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) x;
    (void) n;
    (void) d;
    return 0.f;
}

///////////////////
// HALF SPHERE   //
///////////////////

glm::vec3 HalfSphereDistribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) x;
    auto& normal01 = random.normal01;

    glm::vec3 dir = normal01.sample(random, x ,n);
    if (glm::dot(dir, n) < 0) {
        dir = -1 * dir;
    }
    return dir;
}

float HalfSphereDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) x;
    (void) n;

    if (glm::dot(d, n) < 0) {
        return 0.f;
    }
    return 1.f / (2 * kPI);
}

///////////////////
//    COSINE     //
///////////////////

glm::vec3 CosineDistribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) x;
    auto& normal01 = random.normal01;

    glm::vec3 dir = normal01.sample(random, x ,n);
    dir = dir + n;

    if (glm::dot(dir, n) <= eps) {
        return n;
    }
    if (glm::length(dir) <= 1e-4) {
        return n; // ?!? ?!?!?! ?!? ?!?!?!?? !?!? ?!? ?!?!??! ?!!?
    }
    assert(glm::length(dir) > 1e-4);

    return glm::normalize(dir);
}

float CosineDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) x;
    return std::max(0.f, 1.f / kPI * glm::dot(d, n));
}

///////////////////
//   PRIMITIVE   //
///////////////////

static std::pair<std::optional<intersection_t>, std::optional<intersection_t>> GetPointsForPdf(const Primitive* prim, glm::vec3 x, glm::vec3 d) {
    auto closestInter = prim->Intersect(Ray(x, d));
    if (!closestInter.has_value()) {
        return std::make_pair(std::nullopt, std::nullopt);
    }

    auto [t, normal1, _] = closestInter.value();
    // debug
    if (t <= 1e-8) {
        std::cerr << "GetPointsForPdf unexpected intersection t(" << t << ")" << std::endl;
        return std::make_pair(std::nullopt, std::nullopt);
    }
    //

    static constexpr float eps = 1e-4;
    Point innerPoint = x + (t + eps) * d;

    auto secondInter = prim->Intersect(Ray(innerPoint, d));
    if (!secondInter.has_value()) {
        return std::make_pair(intersection_t{closestInter.value()}, std::nullopt);
    }

    secondInter.value().t += t + eps;
    
    return std::make_pair(
        intersection_t{closestInter.value()}, 
        intersection_t{secondInter.value()}
    );
}

///////////////////
//     BOX       //
///////////////////

BoxDistribution::BoxDistribution(const Primitive* box): box_(box) {}

glm::vec3 BoxDistribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) n;
    auto& uniform01 = random.uniform01;
    auto& rnd = random.rnd;

    auto s = box_->dop_data;
    
    float s_x = s.x;
    float s_y = s.y;
    float s_z = s.z;

    float w_x = s_x * s_x;
    float w_y = s_y * s_y;
    float w_z = s_z * s_z;

here_we_go_again:
    float u = uniform01.sample(rnd);
    float side = (uniform01.sample(rnd) <= 0.5 ? 1 : -1);
    u *= w_x + w_y + w_z;

    float c1 = uniform01.sample(rnd), c2 = uniform01.sample(rnd), c3 = uniform01.sample(rnd);
    c1 = 2 * c1 - 1;
    c2 = 2 * c2 - 1;
    c3 = 2 * c3 - 1;
    Point pnt = {c1 * s_x, c2 * s_y, c3 * s_z};
    if (u < w_x) { // перпенд оси X
        pnt.x = side * s_x;
    } else if (u < w_x + w_y) { // перпенд оси Y
        pnt.y = side * s_y;
    } else { // перпенд оси Z
        pnt.z = side * s_z;
    }

    Point on_box = rotate(box_->rotator, pnt) + box_->pos;

    glm::vec3 sample_ = glm::normalize(on_box - x);
    bool valid_generator = box_->Intersect(Ray{x, sample_}).has_value();
    if (!valid_generator) {
        // ?! ?! ?!
        goto here_we_go_again;
    }

    return sample_;
}

float BoxDistribution::pdfPoint(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const {
    (void) y;

    auto s = box_->dop_data;

    float s_x = s.x;
    float s_y = s.y;
    float s_z = s.z;

    float w_x = s_x * s_x;
    float w_y = s_y * s_y;
    float w_z = s_z * s_z;

    float p_y = 1. / (2 * 4 * (w_x + w_y + w_z));
    return p_y * dist2 / fabs(glm::dot(d, n));
}

float BoxDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    
    auto [inter1, inter2] = GetPointsForPdf(reinterpret_cast<const Primitive*>(box_), x, d);
    if (!inter1.has_value()) {
        //std::cerr << "Box pdf no intersection" << std::endl;
        return 1e-9f;
    }

    auto [t1, normal1, _] =  inter1.value();
    Point closestPoint = x + t1 * d;

    float sum = pdfPoint(glm::distance2(closestPoint, x), closestPoint, normal1, d);

    if (inter2.has_value()) {
        auto [t2, normal2, _] = inter2.value();
        Point otherPoint = x + t2 * d;

        sum += pdfPoint(glm::distance2(otherPoint, x), otherPoint, normal2, d);
    }

    return sum;
}

///////////////////
//   Ellipsoid   //
///////////////////

EllipsoidDistribution::EllipsoidDistribution(const Primitive* ellipsoid): ellipsoid_(ellipsoid) {}

glm::vec3 EllipsoidDistribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) n;
    auto& normal01 = random.normal01;
    
    glm::vec3 r = ellipsoid_->dop_data;
here_we_go_again:
    glm::vec3 k = normal01.sample(random, x, n);

    Point pnt = r * k;

    Point on_ellipsoid = rotate(ellipsoid_->rotator, pnt) + ellipsoid_->pos;

    glm::vec3 sample_ = glm::normalize(on_ellipsoid - x);
    bool valid_generator = ellipsoid_->Intersect(Ray{x, sample_}).has_value();
    if (!valid_generator) {
        // ?! ?! ?!
        goto here_we_go_again;
    }
    
    return sample_;
}

float EllipsoidDistribution::pdfPoint(float dist2, glm::vec3 y, glm::vec3 n_, glm::vec3 d) const {
    glm::vec3 r = ellipsoid_->dop_data;
    glm::vec3 n = rotate(glm::conjugate(ellipsoid_->rotator), y - ellipsoid_->pos) / r;
    float p_y = 1. / (4 * kPI * glm::length(glm::vec3{n.x * r.y * r.z, r.x * n.y * r.z, r.x * r.y * n.z}));
    return p_y * dist2 / fabs(glm::dot(d, n_));
}

float EllipsoidDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    
    auto [inter1, inter2] = GetPointsForPdf(reinterpret_cast<const Primitive*>(ellipsoid_), x, d);
    if (!inter1.has_value()) {
        //std::cerr << "Ellipsoid pdf no intersection" << std::endl;
        return 1e-9f;
    }

    auto [t1, normal1, _] =  inter1.value();
    Point closestPoint = x + t1 * d;

    float sum = pdfPoint(glm::distance2(closestPoint, x), closestPoint, normal1, d);

    if (inter2.has_value()) {
        auto [t2, normal2, _] = inter2.value();
        Point otherPoint = x + t2 * d;

        sum += pdfPoint(glm::distance2(otherPoint, x), otherPoint, normal2, d);
    }

    return sum;
}

///////////////////
//      MIX      //
///////////////////

MixDistribution::MixDistribution(std::vector<std::unique_ptr<Distribution>>&& distribs): distribs_(std::move(distribs)) {}

glm::vec3 MixDistribution::sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) n;
    auto& uniform01 = random.uniform01;
    auto& rnd = random.rnd;

    float flip = uniform01.sample(rnd);
    if (distribs_.empty() || flip <= 0.5f) {
        return cosine_distrib.sample(random, x, n);
    }

    float fid = uniform01.sample(rnd); // float [0,1]
    size_t id = std::floor(fid * distribs_.size()); // int [0,size-1]

    glm::vec3 sample_ = distribs_[id]->sample(random, x, n);
    return sample_;
}

float MixDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    
    float sum = cosine_distrib.pdf(x, n, d);
    if (!distribs_.empty()) {
        float prim_sum = 0.f;
        for(const auto& distrib : distribs_) {
            prim_sum += distrib->pdf(x, n, d);
        }
        prim_sum *= 1.f / distribs_.size();

        sum = 0.5f * sum + 0.5f * prim_sum;
    }
    return sum;
}
