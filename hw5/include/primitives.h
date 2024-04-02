#ifndef DEFINE_PRIMITIVES_H
#define DEFINE_PRIMITIVES_H

#include "point.h"
#include "color.h"
#include "quaternion.h"
#include "materials.h"

#include <algorithm>
#include <optional>
#include <cmath>
#include <iostream>

enum PRIMITIVE_TYPE {
    PLANE     = (1<<0),
    BOX       = (1<<1),
    ELLIPSOID = (1<<2),
    TRIANGLE  = (1<<3)
};

struct intersection_t {
    float t;
    glm::vec3 normal;
    bool interior;
};

class Primitive {
private:
    std::optional<intersection_t> intersectPlane(const Ray &ray, const glm::vec3& n) const;
    std::optional<intersection_t> intersectBox(const Ray &ray, const glm::vec3& s) const;
    std::optional<intersection_t> intersectEllipsoid(const Ray &ray, const glm::vec3& r) const;
    std::optional<intersection_t> intersectTriangle(const Ray &ray, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) const;

public:
    PRIMITIVE_TYPE primitive_type;

    Color col, emission;
    Point pos;
    Quaternion rotator = {1.,0.,0.,0.};
    Material material = Material::DIFFUSE;
    float ior = 0.;

    /*
        Plane     - n
        Box       - s
        Ellipsoid - r
        Triangle  - a
    */
    Point dop_data;

    // Triangle - b, c
    Point dop_data1, dop_data2;

    Primitive(PRIMITIVE_TYPE primitive_type);
    Primitive(PRIMITIVE_TYPE primitive_type, const Point& dop_data);
    Primitive(PRIMITIVE_TYPE triangle_type, const Point& a, const Point& b, const Point& c);
    std::optional<intersection_t> Intersect(const Ray &r) const;
};

/*
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
*/

#endif // DEFINE_PRIMITIVES_H