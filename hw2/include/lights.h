#ifndef DEFINE_LIGHTS_H
#define DEFINE_LIGHTS_H

#include "color.h"
#include "point.h"

#include <color.h>
#include <glm/glm.hpp>

struct point_light_data_t {
    Color color;
    glm::dvec3 dir;
    double dist;
};

class Light {
public:
    Color intens;

    Light() = default;
    Light(const Color& intens);    
    
    virtual point_light_data_t CalcLight(const Point& p) const = 0;
};

class DotLight : public Light {
public:
    Point pos;
    Color att;

    DotLight() = default;
    DotLight(const Color& intens, const Point& pos, const Color& att);
    point_light_data_t CalcLight(const Point& p) const override;
};

class DirectedLight : public Light {
public:
    Point dir;

    DirectedLight() = default;
    DirectedLight(const Color& intens, const Point& dir);
    point_light_data_t CalcLight(const Point& p) const override;
};

#endif // DEFINE_LIGHTS_H