#include "scene.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>

unsigned int get_command(const std::string& command) {
    if (command == "")                  return COMMAND_EMPTY;
    if (command == "DIMENSIONS")        return COMMAND_DIMENSIONS;
    if (command == "BG_COLOR")          return COMMAND_BG_COLOR;
    if (command == "CAMERA_POSITION")   return COMMAND_CAMERA_POSITION;
    if (command == "CAMERA_RIGHT")      return COMMAND_CAMERA_RIGHT;
    if (command == "CAMERA_UP")         return COMMAND_CAMERA_UP;
    if (command == "CAMERA_FORWARD")    return COMMAND_CAMERA_FORWARD;
    if (command == "CAMERA_FOV_X")      return COMMAND_CAMERA_FOV_X;
    if (command == "NEW_PRIMITIVE")     return COMMAND_NEW_PRIMITIVE;
    if (command == "PLANE")             return COMMAND_PLANE;
    if (command == "ELLIPSOID")         return COMMAND_ELLIPSOID;
    if (command == "BOX")               return COMMAND_BOX;
    if (command == "POSITION")          return COMMAND_POSITION;
    if (command == "ROTATION")          return COMMAND_ROTATION;
    if (command == "COLOR")             return COMMAND_COLOR;

    return -1;
}

Camera::Camera(double fov_x) : fov_x(fov_x) {}

Ray Camera::get_to_ray(int x, int y) const {
    double tan_fov_x = tan(fov_x / 2);
    double tan_fov_y = tan_fov_x * height / width;

    float nx =       (2 * (x + 0.5) / width  - 1) * tan_fov_x;
    float ny = 1.f * (2 * (y + 0.5) / height - 1) * tan_fov_y; // not -1.f becase of reversed order in render

    return {pos, nx*right - ny*up + 1.f*forward};
}

Color Scene::raytrace(int x, int y) const {
    Color ans = background;

    double best_dist = -1;
    for (auto &primitive : primitives) {
        auto intersection = primitive->colorIntersect(cam.get_to_ray(x, y));
        if (intersection.has_value()) {
            auto [t, d] = intersection.value();
            if (best_dist == -1 || t < best_dist) {
                best_dist = t;
                ans = d;
            }
        }
    }
    return ans;
}

std::pair<std::unique_ptr<Primitive>, std::string> loadPrimitive(std::istream &in) {
    std::unique_ptr<Primitive> primitive;

    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmdName;
        ss >> cmdName;

        auto cmd = get_command(cmdName);
        if(cmd==COMMAND_EMPTY) break;

        switch (cmd) {
            case COMMAND_ELLIPSOID: {
                Point r;
                ss >> r;
                primitive = std::unique_ptr<Primitive>(new Ellipsoid(r));
                break;
            }
            case COMMAND_PLANE: {
                Point n;
                ss >> n;
                primitive = std::unique_ptr<Primitive>(new Plane(n));
                break;
            }
            case COMMAND_BOX: {
                Point s;
                ss >> s;
                primitive = std::unique_ptr<Primitive>(new Box(s));
                break;
            }
            case COMMAND_COLOR: {
                ss >> primitive->col;
                break;
            }
            case COMMAND_POSITION: {
                ss >> primitive->pos;
                break;
            }
            case COMMAND_ROTATION: {
                ss >> primitive->rotator;
                break;
            }            
            default: {
                std::cerr << "unexpected primitive(" << cmdName << ")" << std::endl;
                return std::make_pair(std::move(primitive), cmdName);
            }
        }
    }
 
    return std::make_pair(std::move(primitive), "");
}

void Scene::load(std::istream &in) {
    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmdName;
        ss >> cmdName;

    pasrse_command_again:
        auto cmd = get_command(cmdName);
        if(cmd==COMMAND_EMPTY) continue;

        switch (cmd) {
            case COMMAND_DIMENSIONS: {
                ss >> cam.width >> cam.height;
                break;
            }
            case COMMAND_BG_COLOR: {
                ss >> background;
                break;
            }
            case COMMAND_CAMERA_POSITION: {
                ss >> cam.pos;
                break;
            }
            case COMMAND_CAMERA_RIGHT: {
                ss >> cam.right;
                break;
            }
            case COMMAND_CAMERA_UP: {
                ss >> cam.up;
                break;
            }
            case COMMAND_CAMERA_FORWARD: { 
                ss >> cam.forward;
                break;
            }
            case COMMAND_CAMERA_FOV_X: {
                ss >> cam.fov_x;
                break;
            }
            case COMMAND_NEW_PRIMITIVE: {
                auto [primitive, restCmd] = loadPrimitive(in);
                primitives.push_back(std::move(primitive));
                cmdName = restCmd;
                if(!cmdName.empty()) {
                    goto pasrse_command_again;
                }
                break;
            }
            default: {
                std::cerr << "unexpected command(" << cmdName << ")" << std::endl;
                break;
            }
        }
    }
}

void Scene::render(std::ostream &out) const {
    out << "P6\n";
    out << cam.width << " " << cam.height << "\n";
    out << 255 << "\n";

    for (int y = 0; y < cam.height; ++y) {
        for (int x = 0; x < cam.width; ++x) {
            uint8_t *rgb = raytrace(x, y).toUInts();
            out.write(reinterpret_cast<char*>(rgb), 3);
        }
    }
}