#ifndef DEFINE_QUATERNION_H
#define DEFINE_QUATERNION_H

#include "point.h"
#include <istream>

class Quaternion {
private:
    Point v;
    double w = 1.f;

public:
    Quaternion() = default;
    Quaternion(double x, double y, double z, double w);
    Quaternion(Point v, double w);

    Quaternion operator+(const Quaternion &q) const;
    Quaternion operator*(const Quaternion &q) const;
    Point transform(const Point &p) const;
    Quaternion conjugate() const;

    friend std::istream &operator>>(std::istream &in, Quaternion &q);
};

std::istream &operator>>(std::istream &in, Quaternion &q);

#endif // DEFINE_QUATERNION_H