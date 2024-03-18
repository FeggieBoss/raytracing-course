#ifndef DEFINE_SCENE_H
#define DEFINE_SCENE_H

#include "color.h"
#include "point.h"
#include "primitives.h"
#include "distributions.h"

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <random>

#define COMMAND_EMPTY              0
#define COMMAND_DIMENSIONS         1
#define COMMAND_BG_COLOR           2
#define COMMAND_CAMERA_POSITION    3
#define COMMAND_CAMERA_RIGHT       4
#define COMMAND_CAMERA_UP          5
#define COMMAND_CAMERA_FORWARD     6
#define COMMAND_CAMERA_FOV_X       7
#define COMMAND_NEW_PRIMITIVE      8
#define COMMAND_PLANE              9
#define COMMAND_ELLIPSOID          10
#define COMMAND_BOX                11
#define COMMAND_POSITION           12
#define COMMAND_ROTATION           13
#define COMMAND_COLOR              14
#define COMMAND_RAY_DEPTH          15
#define COMMAND_METALLIC           16
#define COMMAND_DIELECTRIC         17
#define COMMAND_IOR                18
#define COMMAND_SAMPLES            19
#define COMMAND_EMISSION           20


struct Camera {
    Point pos;
    Point up, right, forward;
    float fov_x;
    unsigned int width, height;

    Camera() = default;
    Camera(float fov_x0);

    Ray GetToRay(float x, float y) const;
};

struct ray_intersection_t {
    intersection_t isec;
    int id;
};

class Scene {
private:
    float eps = 1e-3;
    Uniform01Distribution uniform;

    ray_intersection_t RayIntersection(const Ray& ray) const;
    Color Sample(unsigned int x, unsigned int y);
    Color RayTrace(const Ray& ray, size_t ost_raydepth);
public:
    unsigned int ray_depth;
    unsigned int samples;

    Color background;
    Camera cam;
    std::unique_ptr<MixDistribution> mix_distrib;
    std::vector<std::unique_ptr<Primitive>> primitives;

    Scene() {};

    void Load(std::istream &in);
    void LoadDistribution();
    void Render(std::ostream &out);
};

#endif // DEFINE_SCENE_H