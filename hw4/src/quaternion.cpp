#include "quaternion.h"
#include <math.h>


Point rotate(const Quaternion& q, const Point& x) {
    return q * x;
}

Ray rotate(const Quaternion &q, const Ray& r) {
    return {q * r.o, q * r.d};
} 


std::istream &operator>>(std::istream &in, Quaternion &q) {
    in >> q.x >> q.y >> q.z >> q.w;
    return in;
}