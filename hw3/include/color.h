
#ifndef DEFINE_COLOR_H
#define DEFINE_COLOR_H

#include <istream>
#include <glm/vec3.hpp>

class Color {
public:
    glm::vec3 rgb;
    Color() = default;
    Color(float r, float g, float b);   
    Color(glm::vec3 rgb);    
    float r() const;
    float g() const;
    float b() const;

    unsigned char* toUInts() const;
    friend std::istream& operator>>(std::istream &in, Color &color);
};

Color AcesTonemap(const Color &x);
Color GammaCorrected(const Color &x);

#endif // DEFINE_COLOR_H
