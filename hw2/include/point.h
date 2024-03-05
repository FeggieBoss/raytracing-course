#ifndef DEFINE_POINT_H
#define DEFINE_POINT_H

#include <istream>
#include <glm/vec3.hpp>

#define Point glm::dvec3

class Ray {
public:
    Point o, d;

    Ray() = default;
    Ray(const Point &o, const Point &d);

    Ray operator+(const Point &other) const;
    Ray operator*(double k) const;
};

std::istream& operator>>(std::istream &in, Point &p);
Point operator*(double k, const Point &p);

#endif // DEFINE_POINT_H