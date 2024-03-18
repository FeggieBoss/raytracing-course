#ifndef DEFINE_PRIMITIVES_H
#define DEFINE_PRIMITIVES_H

#include "point.h"
#include "color.h"
#include "quaternion.h"
#include "materials.h"

#include <algorithm>
#include <optional>

enum PRIMITIVE_TYPE {
    PLANE     = (1<<0),
    BOX       = (1<<1),
    ELLIPSOID = (1<<2)
};

struct intersection_t {
    float t;
    glm::vec3 normal;
    bool interior;
};

class Primitive {
private:
    std::optional<intersection_t> intersectPlane(const Ray &r) const;
    std::optional<intersection_t> intersectBox(const Ray &r) const;
    std::optional<intersection_t> intersectEllipsoid(const Ray &r) const;

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
    */
    Point dop_data;

    Primitive(PRIMITIVE_TYPE primitive_type);
    Primitive(PRIMITIVE_TYPE primitive_type, const Point& dop_data);
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