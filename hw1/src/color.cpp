#include "color.h"

#include <cassert>
#include <cmath>

Color::Color(double r, double g, double b): r(r), g(g), b(b) {
    assert(0 <= r && r <= 1 && 0 <= g && g <= 1 && 0 <= b && b <= 1);
}

unsigned char* Color::toUInts() const {
    unsigned char *ans = new unsigned char[3];
    ans[0] = round(255 * r);
    ans[1] = round(255 * g);
    ans[2] = round(255 * b);
    return ans;
}

std::istream& operator>>(std::istream &in, Color &c) {
    in >> c.r >> c.g >> c.b;
    return in;
}