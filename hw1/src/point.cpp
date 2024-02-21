#include "point.h"

#include <cmath>

Point::Point(double x, double y, double z): x(x), y(y), z(z) {}
Point::Point(const Point &p): x(p.x), y(p.y), z(p.z) {}

Point Point::operator+(const Point &p) const {
    return {x+p.x, y+p.y, z+p.z};
}

Point Point::operator-(const Point &p) const {
    return {x-p.x, y-p.y, z-p.z};
}

Point Point::operator/(const Point &p) const {
    return {x/p.x, y/p.y, z/p.z};
}

Point Point::operator*(const Point &p) const {
    return {x*p.x, y*p.y, z*p.z};
}

double Point::operator^(const Point &p) const {
    return x*p.x + y*p.y + z*p.z;
}

Point Point::normalize() const {
    double n = sqrt(len2());
    return {x/n, y/n, z/n};
}

double Point::len2() const {
    return x*x+y*y+z*z;
}

std::istream& operator>>(std::istream &in, Point &p) {
    in >> p.x >> p.y >> p.z;
    return in;
}

Point operator*(double c, const Point &p) {
    return {c*p.x, c*p.y, c*p.z};
}
