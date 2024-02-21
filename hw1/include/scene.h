#ifndef DEFINE_SCENE_H
#define DEFINE_SCENE_H

#include "color.h"
#include "point.h"
#include "primitives.h"

#include <sstream>
#include <string>
#include <vector>
#include <memory>

#define COMMAND_EMPTY           0
#define COMMAND_DIMENSIONS      1
#define COMMAND_BG_COLOR        2
#define COMMAND_CAMERA_POSITION 3
#define COMMAND_CAMERA_RIGHT    4
#define COMMAND_CAMERA_UP       5
#define COMMAND_CAMERA_FORWARD  6
#define COMMAND_CAMERA_FOV_X    7
#define COMMAND_NEW_PRIMITIVE   8
#define COMMAND_PLANE           9
#define COMMAND_ELLIPSOID       10
#define COMMAND_BOX             11
#define COMMAND_POSITION        12
#define COMMAND_ROTATION        13
#define COMMAND_COLOR           14

struct Camera {
    Point pos;
    Point up, right, forward;
    double fov_x;
    int width, height;

    Camera() = default;
    Camera(double fov_x0);

    Ray get_to_ray(int x, int y) const;
};

class Scene {
public:
    Color background;
    Camera cam;
    std::vector<std::unique_ptr<Primitive>> primitives;

    Scene() = default;

    Color raytrace(int x, int y) const;
    void load(std::istream &in);
    void render(std::ostream &out) const;
};

#endif // DEFINE_SCENE_H