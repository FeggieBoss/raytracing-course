#ifndef DEFINE_DISTRIBUTIONS_H
#define DEFINE_DISTRIBUTIONS_H

#include "primitives.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <glm/vec3.hpp>
#include <random>
#include <memory>
#include <variant>

static const float kPI = acos(-1);

struct RANDOM_t {
    std::minstd_rand rnd;
    std::reference_wrapper<std::uniform_real_distribution<float>> uniform01;
    std::reference_wrapper<std::normal_distribution<float>> normal01;
};

enum class DISTRIB_TYPE {
    BOX        = (1<<0),
    ELLIPSOID  = (1<<1),
    MIX        = (1<<2)
};

class Distribution {
private:
    static constexpr float eps = 1e-8;
    static constexpr float eps_big = 1e-3;
    
    // Uniform01
    float SampleUniform01(RANDOM_t& random);
    glm::vec3 SampleUniform01Vec(RANDOM_t& random);
    float PdfUniform01() const;

    // Normal01
    float SampleNormal01(RANDOM_t& random);
    glm::vec3 SampleNormal01Vec(RANDOM_t& random);
    float PdfNormal01() const;

    // HalfSphere
    glm::vec3 SampleHalfSphere(RANDOM_t& random, glm::vec3 x, glm::vec3 n);
    float PdfHalfSphere(glm::vec3 x, glm::vec3 n, glm::vec3 d) const;

    // Cosine
    glm::vec3 SampleCosine(RANDOM_t& random, glm::vec3 x, glm::vec3 n);
    float PdfCosine(glm::vec3 x, glm::vec3 n, glm::vec3 d) const;

    // Box
    glm::vec3 SampleBox(RANDOM_t& random, glm::vec3 x, glm::vec3 n);
    float PdfPointBox(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const;
    float PdfBox(glm::vec3 x, glm::vec3 n, glm::vec3 d) const;

    // Ellipsoid
    glm::vec3 SampleEllipsoid(RANDOM_t& random, glm::vec3 x, glm::vec3 n);
    float PdfPointEllipsoid(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const;
    float PdfEllipsoid(glm::vec3 x, glm::vec3 n, glm::vec3 d) const;

    // Mix
    glm::vec3 SampleMix(RANDOM_t& random, glm::vec3 x, glm::vec3 n);
    float PdfMix(glm::vec3 x, glm::vec3 n, glm::vec3 d) const;
public:
    enum class DATA_T : std::size_t {
        BOX_           = 0,
        ELLIPSOID_     = 1,
        DISTRIBS_      = 2
    };
    std::variant<
        const Primitive*,          // Box
        const Primitive*,          // Ellipsoid
        std::vector<Distribution>  // Mix
    > data;

    DISTRIB_TYPE distrib_type_;

    // HalfSphere // Cosine
    Distribution(DISTRIB_TYPE distrib_type);

    // Box // Ellipsoid
    Distribution(DISTRIB_TYPE box_type, const Primitive* prim);

    // Mix
    Distribution(DISTRIB_TYPE mix_type, std::vector<Distribution>&& distribs);

    // MAIN METHODS
    Distribution() {};
    Distribution(const Distribution& other);
    Distribution& operator=(const Distribution& other);

    glm::vec3 Sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n);
    float Pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const;
};

#endif // DEFINE_DISTRIBUTIONS_H