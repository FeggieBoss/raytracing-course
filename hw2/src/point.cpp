#include "point.h"

#include <cmath>

// RAY
Ray::Ray(const Point &o, const Point &d): o(o), d(d) {}

Ray Ray::operator+(const Point &p) const {
    return {o + p, d};
}
Ray Ray::operator*(double k) const {
    return {k * o, d};
}

std::istream& operator>>(std::istream &in, Point &p) {
    in >> p.x >> p.y >> p.z;
    return in;
}

Point operator*(double c, const Point &p) {
    return {c*p.x, c*p.y, c*p.z};
}
