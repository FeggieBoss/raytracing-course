#ifndef DEFINE_BVH_H
#define DEFINE_BVH_H

#include "primitives.h"
#include <cassert>
#include <iostream>
#include <memory>

constexpr float INF = 1e18;

class AABB_t {
public:
    float areaS = 0.f;
    Point aabb_min;
    Point aabb_max;

    AABB_t();
    AABB_t(const Primitive& prim);
    AABB_t(const AABB_t& other);
    AABB_t& operator=(const AABB_t& other);

    float CalcS();
    void Extend(const Point& p);
    void Extend(const AABB_t& aabb);
    std::optional<intersection_t> Intersect(const Ray &ray) const;
};

struct NODE_t {
    AABB_t aabb;
    uint32_t left_child;
    uint32_t right_child;
    uint32_t first_primitive_id;
    uint32_t primitive_count;
};

class BVH_t {
public:
    std::vector<float> cut_qual;
    std::vector<NODE_t> nodes;

    BVH_t() {};
    BVH_t(std::vector<Primitive>& primitives, uint32_t n);
    ray_intersection_t Intersect(const std::vector<Primitive>& primitives, const Ray &ray, float closest_dist) const;
private:
    uint32_t root_;
    
    uint32_t InitTree(std::vector<Primitive>& primitives, uint32_t first, uint32_t last);
    ray_intersection_t Intersect_(const std::vector<Primitive>& primitives, const Ray &ray, float closest_dist, uint32_t v) const;
};


#endif // DEFINE_BVH_H