#ifndef DEFINE_DISTRIBUTIONS_H
#define DEFINE_DISTRIBUTIONS_H

#include "point.h"
#include "primitives.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <glm/vec3.hpp>
#include <random>
#include <memory>

static const float kPI = acos(-1);

class Distribution {
protected:
    static std::minstd_rand rnd;
public:
    Distribution() {};

    virtual glm::vec3 sample(glm::vec3 x, glm::vec3 n) = 0;
    virtual float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const = 0;
};


class Uniform01Distribution : public Distribution {
private:
    static std::uniform_real_distribution<float> uniform;
public:
    Uniform01Distribution () {};
    
    float sample();
    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class Normal01Distribution : public Distribution  {
private:
    static std::uniform_real_distribution<float> normal;
public:
    Normal01Distribution () {};

    float sample();
    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class HalfSphereDistribution : public Distribution  {
private:
    Normal01Distribution normal01;
    static std::uniform_real_distribution<float> normal;
public:
    HalfSphereDistribution () {};

    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class CosineDistribution : public Distribution  {
private:
    static constexpr float eps = 1e-8;
    Normal01Distribution normal01;
    static std::uniform_real_distribution<float> normal;
public:
    CosineDistribution () {};
    
    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class BoxDistribution : public Distribution {
private:
    static constexpr float eps = 1e-3;

    const Primitive* box_;
    Uniform01Distribution uniform;

    float pdfPoint(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const;
public:
    BoxDistribution(const Primitive* box);

    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class EllipsoidDistribution : public Distribution {
private:
    static constexpr float eps = 1e-3;

    const Primitive* ellipsoid_;
    Normal01Distribution normal;

    float pdfPoint(float dist2, glm::vec3 y, glm::vec3 n, glm::vec3 d) const;
public:
    EllipsoidDistribution(const Primitive* ellipsoid);

    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};


class MixDistribution : public Distribution {
private:
    std::vector<std::unique_ptr<Distribution>> distribs_;
    CosineDistribution cosine_distrib;
    Uniform01Distribution uniform;

public:
    MixDistribution(std::vector<std::unique_ptr<Distribution>>&& distribs);

    glm::vec3 sample(glm::vec3 x, glm::vec3 n) override;
    float pdf(glm::vec3 x, glm::vec3 n, glm::vec3 d) const override;
};

#endif // DEFINE_DISTRIBUTIONS_H