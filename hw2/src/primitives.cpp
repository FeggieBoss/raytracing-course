#include "primitives.h"
#include <cmath>

std::optional<std::pair<intersection_t, Color>> Primitive::colorIntersect(const Ray &r) {
    Ray transformed = transform(glm::conjugate(rotator), (r + -1*pos));
    auto isec = intersect(transformed);
    if (isec.has_value()) {
        auto [t, normal, interior] = isec.value();
        normal = transform(glm::conjugate(rotator), normal);
        normal = glm::normalize(normal);
        return std::make_pair(intersection_t {t, normal, interior}, col);
    }
    return {};
}

// PLANE
Plane::Plane(const Point &n): n(n) {}

std::optional<intersection_t> Plane::intersect(const Ray &r) const {
    double t = -glm::dot(r.o, n) / glm::dot(r.d, n);
    if (t > 0) { 
        if (glm::dot(r.d, n) >= 0) // внутри, тк нормаль соноправлена с лучом
            return intersection_t {t, -1. * n, true};
        return intersection_t {t, n, false};
    }
    return {};  // t < 0 <=> плоскость не видна
}


// BOX
Box::Box(const Point &s): s(s) {}

std::optional<intersection_t> Box::intersect(const Ray &r) const {
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
        } else {
            // if (t1 < 0) // камера находится внутри сферы
            bool interior = t1 < 0;
            double t = (interior ? t2 : t1);

            glm::dvec3 p = r.o + t * r.d;
            glm::dvec3 normal = p / s;
            if (interior) {
                normal = -1. * normal;
            }
            normal = glm::normalize(normal);
            
            double mx = std::max({fabs(normal.x), fabs(normal.y), fabs(normal.z)});
            if (fabs(normal.x) != mx) {
                normal.x = 0;
            }
            if (fabs(normal.y) != mx) {
                normal.y = 0;
            }
            if (fabs(normal.z) != mx) {
                normal.z = 0;
            }
            
            return intersection_t {t, normal, interior};
        }
    }
}

// Ellipsoid
Ellipsoid::Ellipsoid(const Point &r): r(r) {}

std::optional<intersection_t> Ellipsoid::intersect(const Ray &ray) const {
    double a = glm::dot(ray.d / r, ray.d / r);
    double b = 2 * glm::dot(ray.o / r, ray.d / r);
    double c = glm::dot(ray.o / r, ray.o / r) - 1;

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
    } 

    // if (x1 < 0) => inside 
    bool interior = x1 < 0; 
    double t = (interior ? x2 : x1);
    
    glm::dvec3 p = ray.o + t * ray.d;
    glm::dvec3 normal = p / (r * r);
    normal = glm::normalize(normal);
    if (interior) {
        normal = -1. * normal;
    }
    
    return intersection_t {t, normal, interior};
}