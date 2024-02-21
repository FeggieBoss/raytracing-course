#include "primitives.h"
#include <cmath>

std::optional<std::pair<double, Color>> Primitive::colorIntersect(const Ray &r) {
    Ray transformed = (r - pos).rotate(rotator);
    auto result = intersect(transformed);
    if (result.has_value()) {
        return std::make_pair(result.value(), col);
    }
    return {};
}

// RAY
Ray::Ray(const Point &o, const Point &d): o(o), d(d) {}

Ray Ray::operator+(const Point &p) const {
    return {o + p, d};
}

Ray Ray::operator-(const Point &p) const {
    return {o - p, d};
}

Ray Ray::rotate(const Quaternion &q) const {
    return {q.transform(o), q.transform(d)};
} 

// PLANE
Plane::Plane(const Point &n): n(n) {}

std::optional<double> Plane::intersect(const Ray &r) const {
    double t = -(r.o ^ n) / (r.d ^ n);
    if (t < 0) { // плоскость не видна
        return {};
    }
    return t;
}


// BOX
Box::Box(const Point &s): s(s) {}

std::optional<double> Box::intersect(const Ray &r) const {
    Point t1xyz = (-1.f * s - r.o) / r.d;
    Point t2xyz = (       s - r.o) / r.d;

    double t1x = std::min(t1xyz.x, t2xyz.x);
    double t2x = std::max(t1xyz.x, t2xyz.x);

    double t1y = std::min(t1xyz.y, t2xyz.y);
    double t2y = std::max(t1xyz.y, t2xyz.y);

    double t1z = std::min(t1xyz.z, t2xyz.z);
    double t2z = std::max(t1xyz.z, t2xyz.z);

    double t1 = std::max(std::max(t1x, t1y), t1z);
    double t2 = std::min(std::min(t2x, t2y), t2z);

    if (t1 > t2) {
        return {};
    } else {
        if (t2 < 0) { // камера находится снаружи сферы, t2 – точка сзади камеры
            return {};
        } else if (t1 < 0) { // камера находится внутри сферы
            return t2;
        } else { // камера находится снаружи сферы, t1 – ближайшее пересечение
            return t1;
        }
    }
}

// Ellipsoid
Ellipsoid::Ellipsoid(const Point &r): r(r) {}

std::optional<double> Ellipsoid::intersect(const Ray &ray) const {
    double a = (ray.d / r) ^ (ray.d / r);
    double b = 2 * ((ray.o / r) ^ (ray.d / r));
    double c = ((ray.o / r) ^ (ray.o / r)) - 1;

    double d = b * b - 4 * a * c;
    if (d <= 0) {
        return {};
    }

    double x1 = (-b - sqrt(d)) / (2 * a);
    double x2 = (-b + sqrt(d)) / (2 * a);
    if (x1 > x2) {
        std::swap(x1, x2);
    }

    if (x2 < 0) {
        return {};
    } else if (x1 < 0) {
        return x2;
    } else {
        return x1;
    }
}