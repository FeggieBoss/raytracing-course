#include "quaternion.h"
#include <math.h>


Point transform(const Quaternion& q, const Point& x) {
    Quaternion res = (q * Quaternion(0.f, x) * glm::conjugate(q));
    return {res.x, res.y, res.z};
}

Ray transform(const Quaternion &q, const Ray& r) {
    return {transform(q,r.o), transform(q,r.d)};
} 


std::istream &operator>>(std::istream &in, Quaternion &q) {
    in >> q.x >> q.y >> q.z >> q.w;
    return in;
}