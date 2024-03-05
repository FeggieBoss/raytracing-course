#include "lights.h"

Light::Light(const Color& intens): intens(intens) {}

DotLight::DotLight(const Color& intens, const Point& pos, const Color& att): Light(intens), pos(pos), att(att) {}

point_light_data_t DotLight::CalcLight(const Point& p) const {
    Point dir = pos - p;
    double dist = glm::length(dir);
    Color color = 1. / (att.r() + att.g() * dist + att.b() * dist * dist) * intens.rgb;

    return {color, glm::normalize(dir), dist};
}

DirectedLight::DirectedLight(const Color& intens, const Point& dir): Light(intens), dir(dir) {}

point_light_data_t DirectedLight::CalcLight(const Point& p) const {
    (void) p;
    return {intens, glm::normalize(dir), 1e18};
}