#include "color.h"

#include <cassert>
#include <cmath>

Color::Color::Color(double r, double g, double b): rgb(r, g, b) {}
Color::Color::Color(glm::dvec3 rgb): rgb(rgb) {}

double Color::r() const {
    return rgb.r;
}
double Color::g() const {
    return rgb.g;
}
double Color::b() const {
    return rgb.b;
}

static Color saturate(const Color &color) {
    float r = std::max(std::min(1., color.r()), 0.);
    float g = std::max(std::min(1., color.g()), 0.);
    float b = std::max(std::min(1., color.b()), 0.);
    return {r, g, b};
}

Color AcesTonemap(const Color &col) {
    auto x = col.rgb;

    const double a = 2.51f;
    const double b = 0.03f;
    const double c = 2.43f;
    const double d = 0.59f;
    const double e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

Color GammaCorrected(const Color &col) {
    auto x = col.rgb;
    double gamma = 1. / 2.2;
    return {pow(x.r, gamma), pow(x.g, gamma), pow(x.b, gamma)};
}

unsigned char* Color::toUInts() const {
    unsigned char *ans = new unsigned char[3];
    ans[0] = round(255 * r());
    ans[1] = round(255 * g());
    ans[2] = round(255 * b());
    return ans;
}

std::istream& operator>>(std::istream &in, Color &c) {
    in >> c.rgb.r >> c.rgb.g >> c.rgb.b;
    return in;
}