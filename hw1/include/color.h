#ifndef DEFINE_COLOR_H
#define DEFINE_COLOR_H

#include <istream>

class Color {
public:
    double r = 0.f, g = 0.f, b = 0.f;
    Color() = default;
    Color(double r, double g, double b);

    unsigned char* toUInts() const;
    friend std::istream& operator>>(std::istream &in, Color &color);
};

#endif // DEFINE_COLOR_H
