#include "bvh.h"

//////////
// AABB //
//////////

AABB_t::AABB_t() {
    aabb_min = {INF, INF, INF};
    aabb_max = {-INF, -INF, -INF};
}

AABB_t::AABB_t(const AABB_t& other) {
    aabb_min = other.aabb_min;
    aabb_max = other.aabb_max;
}

AABB_t& AABB_t::operator=(const AABB_t& other) {
    aabb_min = other.aabb_min;
    aabb_max = other.aabb_max;
    return *this;
}

float AABB_t::CalcS() {
    glm::vec3 diag = aabb_max - aabb_min;
    areaS = 2.f * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
    return areaS;
}

void AABB_t::Extend(const Point& p) {
    aabb_max.x = std::max(aabb_max.x, p.x);
    aabb_max.y = std::max(aabb_max.y, p.y);
    aabb_max.z = std::max(aabb_max.z, p.z);
    aabb_min.x = std::min(aabb_min.x, p.x);
    aabb_min.y = std::min(aabb_min.y, p.y);
    aabb_min.z = std::min(aabb_min.z, p.z);
}

void AABB_t::Extend(const AABB_t& aabb) {
    Extend(aabb.aabb_max);
    Extend(aabb.aabb_min);
}

AABB_t::AABB_t(const Primitive& prim) {
    switch (prim.primitive_type) {
        case PRIMITIVE_TYPE::BOX: {
            aabb_min = -1.f * prim.dop_data;
            aabb_max = prim.dop_data;
            break;
        }
        case PRIMITIVE_TYPE::ELLIPSOID: {
            aabb_min = -1.f * prim.dop_data;
            aabb_max = prim.dop_data;
            break;
        }
        case PRIMITIVE_TYPE::TRIANGLE: {
            aabb_min = {
                std::min({prim.dop_data.x, prim.dop_data1.x, prim.dop_data2.x}),
                std::min({prim.dop_data.y, prim.dop_data1.y, prim.dop_data2.y}),
                std::min({prim.dop_data.z, prim.dop_data1.z, prim.dop_data2.z})
            };
            aabb_max = {
                std::max({prim.dop_data.x, prim.dop_data1.x, prim.dop_data2.x}),
                std::max({prim.dop_data.y, prim.dop_data1.y, prim.dop_data2.y}),
                std::max({prim.dop_data.z, prim.dop_data1.z, prim.dop_data2.z})
            };
            break;
        }
        default: {
            throw std::runtime_error("AABB_T got bad primitive type in constructor");
            break;
        }
    }

    glm::vec3 oldmn = aabb_min;
    glm::vec3 oldmx = aabb_max;

    aabb_min = {INF, INF, INF};
    aabb_max = {-INF, -INF, -INF};
    for (uint8_t mask = 0; mask < (1<<3); ++mask) {
        glm::vec3 vertex = oldmn;
        for (uint8_t i = 0; i < 3; ++i) {
            vertex[i] = ((mask & (1 << i)) > 0 ? oldmx[i] : oldmn[i]);
        }
        Extend(rotate(prim.rotator, vertex));
    }

    aabb_min = aabb_min + prim.pos;
    aabb_max = aabb_max + prim.pos;
}

std::optional<intersection_t> AABB_t::Intersect(const Ray &ray) const {
    glm::vec3 s = 0.5f * (aabb_max - aabb_min);
    glm::vec3 center = 0.5f * (aabb_max + aabb_min);
    return Primitive::IntersectBox(ray + -1 * center, s);
}

/////////
// BVH //
/////////

BVH_t::BVH_t(std::vector<Primitive>& primitives, uint32_t n) {
    cut_qual.resize(n);
    nodes.reserve(n);
    root_ = InitTree(primitives, 0, n);
}

uint32_t BVH_t::InitTree(std::vector<Primitive>& primitives, uint32_t first, uint32_t last) {
    AABB_t aabb{};
    for (uint32_t i = first; i < last; i++) {
        aabb.Extend(AABB_t{primitives[i]});
    }

    NODE_t cur_node;
    cur_node.aabb = aabb;
    cur_node.first_primitive_id = first;
    cur_node.primitive_count = last - first;
    cur_node.left_child = -1; // 4294967295U
    cur_node.right_child = -1; // 4294967295U

    uint32_t cur_pos = nodes.size();
    nodes.push_back(cur_node);

    if (last - first == 1) {
        return cur_pos;
    }

    // X axis split
    float optimums[3] = {INF, INF, INF};
    uint32_t cuts[3] = {0, 0, 0};
    for (uint8_t axis = 0; axis < 3; ++axis) {
        std::sort(primitives.begin() + first, primitives.begin() + last, [axis](const Primitive& a, const Primitive& b) {
            return a.pos[axis] < b.pos[axis];
        });
        
        // cut_qual[cut] = quality(first, ..., cut - 1) + quality(cut, ..., last - 1)
        std::fill(cut_qual.begin(), cut_qual.begin() + (last - first), 0.f);
        AABB_t pref_aabb{primitives[first]};
        for (uint32_t cut = first + 1; cut < last; ++cut) {
            cut_qual[cut] = pref_aabb.CalcS() * (cut - first);
            pref_aabb.Extend(AABB_t{primitives[cut]});
        }

        AABB_t suf_aabb{};
        for (uint32_t cut = last - 1; cut > first; --cut) {
            suf_aabb.Extend(AABB_t{primitives[cut]});
            cut_qual[cut] += suf_aabb.CalcS() * (last - cut);
        }

        for (uint32_t cut = first + 1; cut < last; ++cut) {
            if (cut_qual[cut] < optimums[axis]) {
                optimums[axis] = cut_qual[cut];
                cuts[axis] = cut;
            }
        }
    }
    

    float optimum = std::min({optimums[0], optimums[1], optimums[2]});
    float without_cut = cur_node.aabb.CalcS() * cur_node.primitive_count;

    // is there need to continue cutting
    if (optimum >= without_cut) {
        return cur_pos;
    }

    // cutting is needed
    uint32_t cut = 0;
    for (uint8_t axis = 0; axis < 3; ++axis) {
        if (optimum == optimums[axis]) {
            std::sort(primitives.begin() + first, primitives.begin() + last, [axis](const Primitive& a, const Primitive& b) {
                return a.pos[axis] < b.pos[axis];
            });
            cut = cuts[axis];
            break;
        }
    }

    cur_node.left_child = InitTree(primitives, first, cut);
    cur_node.right_child = InitTree(primitives, cut, last);
    return cur_pos;
}

ray_intersection_t BVH_t::Intersect(const std::vector<Primitive>& primitives, const Ray &ray, float closest_dist) const {
    return Intersect_(primitives, ray, closest_dist, root_);
}

ray_intersection_t BVH_t::Intersect_(const std::vector<Primitive>& primitives, const Ray &ray, float closest_dist, uint32_t v) const {
    NODE_t cur_node = nodes[v];

    std::optional<intersection_t> isec = cur_node.aabb.Intersect(ray);
    // no intersection
    if (!isec.has_value()) {
        return ray_intersection_t{intersection_t{}, -1};
    }

    auto [t, _, __] = isec.value();
    // already found closer intersection
    if (closest_dist < t) {
        return ray_intersection_t{intersection_t{}, -1};
    }

    ray_intersection_t ray_isec;
    ray_isec.isec.t = INF;
    ray_isec.id = -1;

    // checking if this node is a leaf (in our case - node either have both childs or none)
    if (cur_node.left_child == (uint32_t)-1) { // == 4294967295U
        for (uint32_t i = cur_node.first_primitive_id; i < cur_node.first_primitive_id + cur_node.primitive_count; ++i) {
            auto isec = primitives[i].Intersect(ray);
            if (isec.has_value() && isec.value().t < ray_isec.isec.t) {
                ray_isec = ray_intersection_t{isec.value(), (int)i};
            }
        }
        return ray_isec;
    }

    ray_intersection_t isecl = Intersect_(primitives, ray, closest_dist, cur_node.left_child);
    if (isecl.id != -1 && isecl.isec.t < ray_isec.isec.t) {
        closest_dist = isecl.isec.t;
        ray_isec = isecl;
    }
    ray_intersection_t isecr = Intersect_(primitives, ray, closest_dist, cur_node.right_child);
    if (isecr.id != -1 && isecr.isec.t < ray_isec.isec.t) {
        ray_isec = isecr;
    }
    return ray_isec;
}