#include "color.h"

#include <cassert>
#include <cmath>

Color::Color::Color(float r, float g, float b): rgb(r, g, b) {}
Color::Color::Color(glm::vec3 rgb): rgb(rgb) {}

float Color::r() const {
    return rgb.r;
}
float Color::g() const {
    return rgb.g;
}
float Color::b() const {
    return rgb.b;
}

static Color saturate(const Color &color) {
    float r = std::max(std::min(1.f, color.r()), 0.f);
    float g = std::max(std::min(1.f, color.g()), 0.f);
    float b = std::max(std::min(1.f, color.b()), 0.f);
    return {r, g, b};
}

Color AcesTonemap(const Color &col) {
    auto x = col.rgb;

    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

Color GammaCorrected(const Color &col) {
    auto x = col.rgb;
    float gamma = 1. / 2.2;
    return {std::pow(x.r, gamma), std::pow(x.g, gamma), std::pow(x.b, gamma)};
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