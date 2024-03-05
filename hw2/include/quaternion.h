#ifndef DEFINE_QUATERNION_H
#define DEFINE_QUATERNION_H

#include "point.h"

#include <istream>
#include <glm/gtc/quaternion.hpp>

#define Quaternion glm::dquat

Point transform(const Quaternion& q, const Point& x);
Ray transform(const Quaternion &q, const Ray& r);

std::istream &operator>>(std::istream &in, Quaternion &q);

#endif // DEFINE_QUATERNION_H