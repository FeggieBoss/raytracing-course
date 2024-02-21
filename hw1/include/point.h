#ifndef DEFINE_POINT_H
#define DEFINE_POINT_H

#include <istream>

class Point {
public:
    double x = 0.f, y = 0.f, z = 0.f;
    Point() = default;
    Point(const Point &p);
    Point(double x, double y, double z);

    Point  operator+(const Point &other) const;
    Point  operator-(const Point &other) const;
    Point  operator/(const Point &other) const;
    Point  operator*(const Point &other) const;
    double operator^(const Point &other) const;
    Point normalize() const;
    double len2() const;

    friend std::istream& operator>>(std::istream &in, Point &p);
};
Point operator*(double k, const Point &p);

#endif // DEFINE_POINT_H