#ifndef DEFINE_DISTRIBUTIONS_H
#define DEFINE_DISTRIBUTIONS_H

#include "primitives.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <glm/vec3.hpp>
#include <random>
#include <memory>

static const float kPI = acos(-1);

struct RANDOM_t;

class Distribution {
public:
    Distribution() {};

    virtual glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) = 0;
    virtual float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const = 0;
};


class Uniform01Distribution : public Distribution {
private:
    std::uniform_real_distribution<float> uniform;
public:
    Uniform01Distribution () { uniform = std::uniform_real_distribution<float>{0.f, 1.f}; };
    
    float sample(std::minstd_rand& rnd);
    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class Normal01Distribution : public Distribution  {
private:
    std::normal_distribution<float> normal;
public:
    Normal01Distribution () { normal = std::normal_distribution<float>{0.f, 1.f}; };

    float sample(std::minstd_rand& rnd);
    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};



struct RANDOM_t {
    std::minstd_rand rnd;
    Uniform01Distribution uniform01;
    Normal01Distribution normal01; 
};


class HalfSphereDistribution : public Distribution  {
public:
    HalfSphereDistribution () {};

    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class CosineDistribution : public Distribution  {
private:
    static constexpr float eps = 1e-8;
public:
    CosineDistribution () {};
    
    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class BoxDistribution : public Distribution {
private:
    static constexpr float eps = 1e-3;

    const Primitive* box_;

    float pdfPoint(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const;
public:
    BoxDistribution(const Primitive* box);

    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class EllipsoidDistribution : public Distribution {
private:
    static constexpr float eps = 1e-3;

    const Primitive* ellipsoid_;

    float pdfPoint(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const;
public:
    EllipsoidDistribution(const Primitive* ellipsoid);

    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class MixDistribution : public Distribution {
private:
    std::vector<std::unique_ptr<Distribution>> distribs_;
    CosineDistribution cosine_distrib;

public:
    MixDistribution(std::vector<std::unique_ptr<Distribution>>&& distribs);

    glm::vec3 sample(RANDOM_t& random, glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};

#endif // DEFINE_DISTRIBUTIONS_H