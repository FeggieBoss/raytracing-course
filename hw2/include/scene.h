#ifndef DEFINE_SCENE_H
#define DEFINE_SCENE_H

#include "color.h"
#include "point.h"
#include "primitives.h"
#include "lights.h"

#include <sstream>
#include <string>
#include <vector>
#include <memory>

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
#define COMMAND_AMBIENT_LIGHT      16
#define COMMAND_NEW_LIGHT          17
#define COMMAND_LIGHT_INTENSITY    18
#define COMMAND_LIGHT_DIRECTION    19
#define COMMAND_LIGHT_POSITION     20
#define COMMAND_LIGHT_ATTENUATION  21
#define COMMAND_METALLIC           22
#define COMMAND_DIELECTRIC         23
#define COMMAND_IOR                24


struct Camera {
    Point pos;
    Point up, right, forward;
    float fov_x;
    int width, height;

    Camera() = default;
    Camera(float fov_x0);

    Ray GetToRay(int x, int y) const;
};

struct ray_intersection_t {
    intersection_t isec;
    int id;
};

class Scene {
private:
    float eps = 1e-3;
    ray_intersection_t RayIntersection(const Ray& ray, float tmax) const;
    Color RayTrace(const Ray& ray, size_t ost_raydepth) const;
public:
    unsigned int ray_depth;
    Color ambient_light;

    Color background;
    Camera cam;
    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Light>> lights;

    Scene() = default;

    void Load(std::istream &in);
    void Render(std::ostream &out) const;
};

#endif // DEFINE_SCENE_H