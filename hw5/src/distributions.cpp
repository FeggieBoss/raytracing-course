#include "distributions.h"

//////////////////
// MAIN METHODS //
//////////////////

Distribution::Distribution(const Distribution& other) {
    distrib_type_ = other.distrib_type_;
    data = other.data;
}
Distribution& Distribution::operator=(const Distribution& other) {
    distrib_type_ = other.distrib_type_;
    data = other.data;
    return *this;
}

glm::vec3 Distribution::Sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    glm::vec3 ret;

    switch (distrib_type_) {
        case DISTRIB_TYPE::BOX: {
            ret = SampleBox(random, x, n);
            break;
        }
        case DISTRIB_TYPE::ELLIPSOID: {
            ret = SampleEllipsoid(random, x, n);
            break;
        }
        case DISTRIB_TYPE::MIX: {
            ret = SampleMix(random, x, n);
            break;
        }
    
        default: {
            throw std::invalid_argument("Distribution::sample: unexpected DISTRIB_TYPE");
            break;
        }
    }

    return ret;
}

float Distribution::Pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    float ret;

    switch (distrib_type_) {
        case DISTRIB_TYPE::BOX: {
            ret = PdfBox(x, n, d);
            break;
        }
        case DISTRIB_TYPE::ELLIPSOID: {
            ret = PdfEllipsoid(x, n, d);
            break;
        }
        case DISTRIB_TYPE::MIX: {
            ret = PdfMix(x, n, d);
            break;
        }
    
        default: {
            throw std::invalid_argument("Distribution::sample: unexpected DISTRIB_TYPE");
            break;
        }
    }

    return ret;
}

///////////////////
//   UNIFORM01   //
///////////////////

float Distribution::SampleUniform01(RANDOM_t& random) {
    std::uniform_real_distribution<float>& uniform = random.uniform01.get();
    auto& rnd = random.rnd;

    return uniform(rnd);
}

glm::vec3 Distribution::SampleUniform01Vec(RANDOM_t& random) {
    std::uniform_real_distribution<float>& uniform = random.uniform01.get();
    auto& rnd = random.rnd;

    return glm::normalize(glm::vec3{uniform(rnd), uniform(rnd), uniform(rnd)});
}

float Distribution::PdfUniform01() const {
    return 0.f;
}

///////////////////
//   NORMAL01    //
///////////////////

float Distribution::SampleNormal01(RANDOM_t& random) {
    std::normal_distribution<float>& normal = random.normal01.get();
    auto& rnd = random.rnd;

    return normal(rnd);
}

glm::vec3 Distribution::SampleNormal01Vec(RANDOM_t& random) {
    std::normal_distribution<float>& normal = random.normal01.get();
    auto& rnd = random.rnd;

    float flip1 = normal(rnd);
    float flip2 = normal(rnd);
    float flip3 = normal(rnd);
    return glm::normalize(glm::vec3{flip1, flip2, flip3});
}

float Distribution::PdfNormal01() const {
    return 0.f;
}

///////////////////
//  HALF SPHERE  //
///////////////////

glm::vec3 Distribution::SampleHalfSphere(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) x;

    glm::vec3 dir = SampleNormal01Vec(random);
    if (glm::dot(dir, n) < 0) {
        dir = -1 * dir;
    }
    return dir;
}

float Distribution::PdfHalfSphere(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
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

glm::vec3 Distribution::SampleCosine(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) x;

    glm::vec3 dir = SampleNormal01Vec(random);
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

float Distribution::PdfCosine(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
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

// Box or Ellipsoid
Distribution::Distribution(DISTRIB_TYPE distrib_type, const Primitive* prim) {
    distrib_type_ = distrib_type;
    assert(distrib_type == DISTRIB_TYPE::BOX || distrib_type == DISTRIB_TYPE::ELLIPSOID);

    switch (prim->primitive_type) {
        case PRIMITIVE_TYPE::BOX: {
            data.emplace<static_cast<std::size_t>(DATA_T::BOX_)>(prim);
            assert(distrib_type == DISTRIB_TYPE::BOX);
            break;
        }
        case PRIMITIVE_TYPE::ELLIPSOID: {
            data.emplace<static_cast<std::size_t>(DATA_T::ELLIPSOID_)>(prim);
            assert(distrib_type == DISTRIB_TYPE::ELLIPSOID);
            break;
        }
        default: {
            throw std::invalid_argument("Distribution constructor BOX/Ellipsoid: unexpected prim type");
            break;
        }
    }
}

glm::vec3 Distribution::SampleBox(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) n;
    const Primitive* box_ = std::get<static_cast<std::size_t>(DATA_T::BOX_)>(data);

    auto s = box_->dop_data;
    
    float s_x = s.x;
    float s_y = s.y;
    float s_z = s.z;

    float w_x = s_x * s_x;
    float w_y = s_y * s_y;
    float w_z = s_z * s_z;

here_we_go_again:
    float u = SampleUniform01(random);
    float side = (SampleUniform01(random) <= 0.5 ? 1 : -1);
    u *= w_x + w_y + w_z;

    float c1 = SampleUniform01(random), c2 = SampleUniform01(random), c3 = SampleUniform01(random);
    c1 = 2 * c1 - 1;
    c2 = 2 * c2 - 1;
    c3 = 2 * c3 - 1;
    Point pnt = {c1 * s_x, c2 * s_y, c3 * s_z};
    if (u < w_x) {  // перпенд оси X
        pnt.x = side * s_x;
    } else if (u < w_x + w_y) {  // перпенд оси Y
        pnt.y = side * s_y;
    } else {  // перпенд оси Z
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

float Distribution::PdfPointBox(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const {
    (void) y;
    const Primitive* box_ = std::get<static_cast<std::size_t>(DATA_T::BOX_)>(data);

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

float Distribution::PdfBox(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    const Primitive* box_ = std::get<static_cast<std::size_t>(DATA_T::BOX_)>(data);

    auto [inter1, inter2] = GetPointsForPdf(box_, x, d);
    if (!inter1.has_value()) {
        //std::cerr << "Box pdf no intersection" << std::endl;
        return 1e-9f;
    }

    auto [t1, normal1, _] =  inter1.value();
    Point closestPoint = x + t1 * d;

    float sum = PdfPointBox(glm::distance2(closestPoint, x), closestPoint, normal1, d);

    if (inter2.has_value()) {
        auto [t2, normal2, _] = inter2.value();
        Point otherPoint = x + t2 * d;

        sum += PdfPointBox(glm::distance2(otherPoint, x), otherPoint, normal2, d);
    }

    return sum;
}

///////////////////
//   Ellipsoid   //
///////////////////

glm::vec3 Distribution::SampleEllipsoid(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) n;
    const Primitive* ellipsoid_ = std::get<static_cast<std::size_t>(DATA_T::ELLIPSOID_)>(data);
    
    glm::vec3 r = ellipsoid_->dop_data;
here_we_go_again:
    glm::vec3 k = SampleNormal01Vec(random);

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

float Distribution::PdfPointEllipsoid(float dist2, glm::vec3 y, glm::vec3 n_, glm::vec3 d) const {
    const Primitive* ellipsoid_ = std::get<static_cast<std::size_t>(DATA_T::ELLIPSOID_)>(data);

    glm::vec3 r = ellipsoid_->dop_data;
    glm::vec3 n = rotate(glm::conjugate(ellipsoid_->rotator), y - ellipsoid_->pos) / r;
    float p_y = 1. / (4 * kPI * glm::length(glm::vec3{n.x * r.y * r.z, r.x * n.y * r.z, r.x * r.y * n.z}));
    return p_y * dist2 / fabs(glm::dot(d, n_));
}

float Distribution::PdfEllipsoid(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    const Primitive* ellipsoid_ = std::get<static_cast<std::size_t>(DATA_T::ELLIPSOID_)>(data);
    
    auto [inter1, inter2] = GetPointsForPdf(ellipsoid_, x, d);
    if (!inter1.has_value()) {
        //std::cerr << "Ellipsoid pdf no intersection" << std::endl;
        return 1e-9f;
    }

    auto [t1, normal1, _] =  inter1.value();
    Point closestPoint = x + t1 * d;

    float sum = PdfPointEllipsoid(glm::distance2(closestPoint, x), closestPoint, normal1, d);

    if (inter2.has_value()) {
        auto [t2, normal2, _] = inter2.value();
        Point otherPoint = x + t2 * d;

        sum += PdfPointEllipsoid(glm::distance2(otherPoint, x), otherPoint, normal2, d);
    }

    return sum;
}

///////////////////
//      MIX      //
///////////////////

Distribution::Distribution(DISTRIB_TYPE mix_type, std::vector<Distribution>&& distribs) {
    assert(mix_type == DISTRIB_TYPE::MIX);
    distrib_type_ = DISTRIB_TYPE::MIX;

    data.emplace<static_cast<std::size_t>(DATA_T::DISTRIBS_)>(std::move(distribs));
}

glm::vec3 Distribution::SampleMix(RANDOM_t& random, glm::vec3 x, glm::vec3 n) {
    (void) n;
    std::vector<Distribution>& distribs_ = std::get<static_cast<std::size_t>(DATA_T::DISTRIBS_)>(data);

    float flip = SampleUniform01(random);
    if (distribs_.empty() || flip <= 0.5f) {
        return SampleCosine(random, x, n);
    }

    float fid = SampleUniform01(random);  // float [0,1]
    size_t id = std::floor(fid * distribs_.size());  // int [0,size-1]

    glm::vec3 sample_ = distribs_[id].Sample(random, x, n);
    return sample_;
}

float Distribution::PdfMix(glm::vec3 x, glm::vec3 n, glm::vec3 d) const {
    (void) n;
    const std::vector<Distribution>& distribs_ = std::get<static_cast<std::size_t>(DATA_T::DISTRIBS_)>(data);
    
    float sum = PdfCosine(x, n, d);
    if (!distribs_.empty()) {
        float prim_sum = 0.f;
        for(const auto& distrib : distribs_) {
            prim_sum += distrib.Pdf(x, n, d);
        }
        prim_sum *= 1.f / distribs_.size();

        sum = 0.5f * sum + 0.5f * prim_sum;
    }
    return sum;
}
