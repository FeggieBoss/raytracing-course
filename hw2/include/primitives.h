#ifndef DEFINE_PRIMITIVES_H
#define DEFINE_PRIMITIVES_H

#include "point.h"
#include "color.h"
#include "quaternion.h"
#include "materials.h"

#include <algorithm>
#include <optional>

struct intersection_t {
    float t;
    glm::vec3 normal;
    bool interior;
};

class Primitive {
private:
    virtual std::optional<intersection_t> intersect(const Ray &r) const = 0;

public:
    Color col;
    Point pos;
    Quaternion rotator = {1.,0.,0.,0.};
    Material material = Material::DIFFUSE;
    float ior = 0.;

    Primitive() = default;
    std::optional<std::pair<intersection_t, Color>> colorIntersect(const Ray &r);
};

class Plane : public Primitive {
private:
    std::optional<intersection_t> intersect(const Ray &r) const override;

public:
    Point n;

    Plane() = default;
    Plane(const Point &n);
};

class Box : public Primitive {
private:
    std::optional<intersection_t> intersect(const Ray &r) const override;

public:
    Point s;

    Box() = default;
    Box(const Point &s);
};


class Ellipsoid : public Primitive {
private:
    std::optional<intersection_t> intersect(const Ray &r) const override;

public:
    Point r;

    Ellipsoid() = default;
    Ellipsoid(const Point &r);
};

#endif // DEFINE_PRIMITIVES_H