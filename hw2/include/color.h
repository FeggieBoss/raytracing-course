
#ifndef DEFINE_COLOR_H
#define DEFINE_COLOR_H

#include <istream>
#include <glm/vec3.hpp>

class Color {
public:
    glm::dvec3 rgb;
    Color() = default;
    Color(double r, double g, double b);   
    Color(glm::dvec3 rgb);    
    double r() const;
    double g() const;
    double b() const;

    unsigned char* toUInts() const;
    friend std::istream& operator>>(std::istream &in, Color &color);
};

Color AcesTonemap(const Color &x);
Color GammaCorrected(const Color &x);

#endif // DEFINE_COLOR_H
