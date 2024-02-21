#ifndef DEFINE_PRIMITIVES_H
#define DEFINE_PRIMITIVES_H

#include "point.h"
#include "color.h"
#include "quaternion.h"

#include <optional>

class Ray {
public:
    Point o, d;

    Ray() = default;
    Ray(const Point &o, const Point &d);

    Ray operator+(const Point &other) const;
    Ray operator-(const Point &other) const;
    Ray rotate(const Quaternion &q) const;
};

class Primitive {
private:
    virtual std::optional<double> intersect(const Ray &r) const = 0;

public:
    Color col;
    Point pos;
    Quaternion rotator;

    Primitive() = default;
    std::optional<std::pair<double, Color>> colorIntersect(const Ray &r);
};

class Plane : public Primitive {
private:
    std::optional<double> intersect(const Ray &r) const override;

public:
    Point n;

    Plane() = default;
    Plane(const Point &n);
};

class Box : public Primitive {
private:
    std::optional<double> intersect(const Ray &r) const override;

public:
    Point s;

    Box() = default;
    Box(const Point &s);
};


class Ellipsoid : public Primitive {
private:
    std::optional<double> intersect(const Ray &r) const override;

public:
    Point r;

    Ellipsoid() = default;
    Ellipsoid(const Point &r);
};

#endif // DEFINE_PRIMITIVES_H