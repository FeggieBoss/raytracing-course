#include "primitives.h"

Primitive::Primitive(PRIMITIVE_TYPE primitive_type): primitive_type(primitive_type) {}
Primitive::Primitive(PRIMITIVE_TYPE primitive_type, const Point& dop_data): primitive_type(primitive_type), dop_data(dop_data) {}

std::optional<intersection_t> Primitive::Intersect(const Ray &r) const {
    Ray rotated = rotate(glm::conjugate(rotator), (r + -1*pos));
    
    std::optional<intersection_t> isec = std::nullopt;
    switch (primitive_type) {
        case PRIMITIVE_TYPE::PLANE: {
            isec = intersectPlane(rotated);
            break;
        }
        case PRIMITIVE_TYPE::BOX: {
            isec = intersectBox(rotated);
            break;
        }
        case PRIMITIVE_TYPE::ELLIPSOID: {
            isec = intersectEllipsoid(rotated);
            break;
        }
        
        default: {
            std::cerr << "unexpected primitive type(" << primitive_type << ") in intersection" << std::endl;
            exit(1);
            break;
        }
    }
    
    if (isec.has_value()) {
        auto [t, normal, interior] = isec.value();
        normal = rotate(rotator, normal);

        assert(glm::length(normal) < 1 + 1e-5);
        
        return intersection_t{t, normal, interior};
    }
    return {};
}

// PLANE
std::optional<intersection_t> Primitive::intersectPlane(const Ray &r) const {
    auto n = dop_data;

    float t = -glm::dot(r.o, n) / glm::dot(r.d, n);
    if (t > 1e5) {
        return {};
    }
    if (t > 0) { 
        if (glm::dot(r.d, n) >= 0) // внутри, тк нормаль соноправлена с лучом
            return intersection_t {t, -1. * n, true};
        return intersection_t {t, n, false};
    }
    return {};  // t < 0 <=> плоскость не видна
}


// BOX
std::optional<intersection_t> Primitive::intersectBox(const Ray &r) const {
    auto s = dop_data;
    
    Point t1xyz = (-1.f * s - r.o) / r.d;
    Point t2xyz = (       s - r.o) / r.d;

    float t1x = std::min(t1xyz.x, t2xyz.x);
    float t2x = std::max(t1xyz.x, t2xyz.x);

    float t1y = std::min(t1xyz.y, t2xyz.y);
    float t2y = std::max(t1xyz.y, t2xyz.y);

    float t1z = std::min(t1xyz.z, t2xyz.z);
    float t2z = std::max(t1xyz.z, t2xyz.z);

    float t1 = std::max(std::max(t1x, t1y), t1z);
    float t2 = std::min(std::min(t2x, t2y), t2z);

    if (t1 > t2) {
        return {};
    } else {
        if (t2 < 0) { // камера находится снаружи сферы, t2 – точка сзади камеры
            return {};
        } else {
            // if (t1 < 0) // камера находится внутри сферы
            bool interior = t1 < 0;
            float t = (interior ? t2 : t1);

            glm::vec3 p = r.o + t * r.d;
            glm::vec3 normal = p / s;
            if (interior) {
                normal = -1. * normal;
            }
            
            float mx = std::max({fabs(normal.x), fabs(normal.y), fabs(normal.z)});
            if (fabs(normal.x) != mx) {
                normal.x = 0;
            }
            if (fabs(normal.y) != mx) {
                normal.y = 0;
            }
            if (fabs(normal.z) != mx) {
                normal.z = 0;
            }
            
            normal = glm::normalize(normal);
            return intersection_t {t, normal, interior};
        }
    }
}

// Ellipsoid
std::optional<intersection_t> Primitive::intersectEllipsoid(const Ray &ray) const {
    auto r = dop_data;

    float a = glm::dot(ray.d / r, ray.d / r);
    float b = 2 * glm::dot(ray.o / r, ray.d / r);
    float c = glm::dot(ray.o / r, ray.o / r) - 1;

    float d = b * b - 4 * a * c;
    if (d <= 0) {
        return {};
    }

    float x1 = (-b - sqrt(d)) / (2 * a);
    float x2 = (-b + sqrt(d)) / (2 * a);
    if (x1 > x2) {
        std::swap(x1, x2);
    }

    if (x2 < 0) {
        return {};
    } 

    // if (x1 < 0) => inside 
    bool interior = x1 < 0; 
    float t = (interior ? x2 : x1);
    
    glm::vec3 p = ray.o + t * ray.d;
    glm::vec3 normal = p / (r * r);
    normal = glm::normalize(normal);
    if (interior) {
        normal = -1. * normal;
    }
    
    return intersection_t {t, normal, interior};
}