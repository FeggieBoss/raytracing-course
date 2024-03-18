#include "distributions.h"

std::minstd_rand Distribution::rnd;
std::uniform_real_distribution<float> Uniform01Distribution::uniform(0,1);
std::uniform_real_distribution<float> Normal01Distribution::normal(0,1);

///////////////////
//    UNIFORM    //
///////////////////

float Uniform01Distribution::sample() {
    return uniform(rnd);
}

glm::vec3 Uniform01Distribution::sample(glm::vec3 x, glm::vec3 n) {
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

float Normal01Distribution::sample() {
    return normal(rnd);
}

glm::vec3 Normal01Distribution::sample(glm::vec3 x, glm::vec3 n) {
    (void) x;
    (void) n;
    return glm::normalize(glm::vec3{normal(rnd), normal(rnd), normal(rnd)});
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

glm::vec3 HalfSphereDistribution::sample(glm::vec3 x, glm::vec3 n) {
    (void) x;

    glm::vec3 dir = normal01.sample(x ,n);
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

glm::vec3 CosineDistribution::sample(glm::vec3 x, glm::vec3 n) {
    (void) x;

    glm::vec3 dir = normal01.sample(x ,n);
    dir = dir + n;
    if (glm::dot(dir, n) <= eps) {
        return n;
    }
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
    auto closestInter = prim->colorIntersect(Ray(x, d));
    if (!closestInter.has_value()) {
        return std::make_pair(std::nullopt, std::nullopt);
    }

    auto [t, normal1, _] = closestInter.value().first;
    // debug
    if (t <= 1e-8) {
        exit(1);
    }
    //

    Point innerPoint = x + (t + 1e-3) * d;

    auto secondInter = prim->colorIntersect(Ray(innerPoint, d));
    if (!secondInter.has_value()) {
        return std::make_pair(
            std::optional<intersection_t>{closestInter.value().first}, 
            std::nullopt
        );
    }

    secondInter.value().first.t += t + 1e-3;
    
    return std::make_pair(
        std::optional<intersection_t>{closestInter.value().first}, 
        std::optional<intersection_t>{secondInter.value().first}
    );
}

///////////////////
//     BOX       //
///////////////////

BoxDistribution::BoxDistribution(const Box* box): box_(box) {}

glm::vec3 BoxDistribution::sample(glm::vec3 x, glm::vec3 n) {
    (void) n;
    
    float s_x = box_->s.x;
    float s_y = box_->s.y;
    float s_z = box_->s.z;

    float w_x = s_x * s_x;
    float w_y = s_y * s_y;
    float w_z = s_z * s_z;

    float u = uniform.sample();
    float side = (uniform.sample() <= 0.5 ? 1 : -1);
    u *= w_x + w_y + w_z;

    float c1 = uniform.sample(), c2 = uniform.sample(), c3 = uniform.sample();
    c1 = 2 * c1 - 1;
    c2 = 2 * c2 - 1;
    c3 = 2 * c3 - 1;
    Point pnt = {c1 * s_x, c2 * s_y, c3 * s_z};
    if (u < w_x) { // перпенд оси X
        pnt.x = side * s_x;
    } else if (u < w_x + w_z) { // перпенд оси Y
        pnt.y = side * s_y;
    } else { // перпенд оси Z
        pnt.z = side * s_z;
    }

    Point on_box = transform(glm::conjugate(box_->rotator), pnt) + box_->pos;
    return glm::normalize(on_box - x);
}

float BoxDistribution::pdfPoint(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const {
    (void) y;

    float s_x = box_->s.x;
    float s_y = box_->s.y;
    float s_z = box_->s.z;

    float w_x = s_x * s_x;
    float w_y = s_y * s_y;
    float w_z = s_z * s_z;

    float p_y = 1. / (2 * 4 * (w_x + w_y + w_z));
    return p_y * (dist2 / fabs(glm::dot(d, n)));
}

float BoxDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    
    auto [inter1, inter2] = GetPointsForPdf(reinterpret_cast<const Primitive*>(box_), x, d);
    if (!inter1.has_value()) {
        return 0.f;
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

EllipsoidDistribution::EllipsoidDistribution(const Ellipsoid* ellipsoid): ellipsoid_(ellipsoid) {}

glm::vec3 EllipsoidDistribution::sample(glm::vec3 x, glm::vec3 n) {
    (void) n;
    
    glm::vec3 r = ellipsoid_->r;
    glm::vec3 k = normal.sample(x, n);

    Point pnt = r * k;

    Point on_ellipsoid = transform(glm::conjugate(ellipsoid_->rotator), pnt) + ellipsoid_->pos;
    return glm::normalize(on_ellipsoid - x);
}

float EllipsoidDistribution::pdfPoint(float dist2, glm::vec3 y, glm::vec3 n_, glm::vec3 d) const {
    glm::vec3 r = ellipsoid_->r;
    glm::vec3 n = transform(ellipsoid_->rotator, y - ellipsoid_->pos) / r;
    float p_y = 1. / (4 * kPI * glm::length(glm::vec3{n.x * r.y * r.z, r.x * n.y * r.z, r.x * r.y * n.z}));
    return p_y * (dist2 / fabs(glm::dot(d, n_)));
}

float EllipsoidDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    
    auto [inter1, inter2] = GetPointsForPdf(reinterpret_cast<const Primitive*>(ellipsoid_), x, d);
    if (!inter1.has_value()) {
        return 0.f;
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

glm::vec3 MixDistribution::sample(glm::vec3 x, glm::vec3 n) {
    (void) n;
    
    float fid = uniform.sample(); // float [0,1]
    size_t id = std::floor(fid * distribs_.size()); // int [0,size-1]

    glm::vec3 sample_ = distribs_[id]->sample(x, n);
    return sample_;
}

float MixDistribution::pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    
    float sum = 0;
    for(const auto& distrib : distribs_) {
        sum += distrib->pdf(x, n, d);
    }
    sum /= 1. * distribs_.size();

    return sum;
}