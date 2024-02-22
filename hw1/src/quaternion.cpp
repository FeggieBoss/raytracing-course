#include "quaternion.h"
#include <math.h>

Quaternion::Quaternion(double x, double y, double z, double w): v{x, y, z}, w(w) {}
Quaternion::Quaternion(Point v, double w): v(v), w(w) {}

Quaternion Quaternion::operator + (const Quaternion &q) const {
    return {v + q.v, w + q.w};
}

Quaternion Quaternion::operator*(const Quaternion &q) const {
    return {w*q.v + q.w*v + (v*q.v), w*q.w - (v^q.v)};
}

Quaternion Quaternion::conjugate() const {
    return {-1.f * v, w};
}

Point Quaternion::transform(const Point &x) const {
    return ((*this) * Quaternion(x, 0.f) * conjugate()).v;
}

std::istream &operator>>(std::istream &in, Quaternion &q) {
    in >> q.v >> q.w;
    return in;
}