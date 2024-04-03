#include "scene.h"
#include <fstream>

///////////////////
// SCENE LOADING //
///////////////////

unsigned int get_command(const std::string& command) {
    if (command == "")                      return COMMAND_EMPTY;
    if (command == "DIMENSIONS")            return COMMAND_DIMENSIONS;
    if (command == "BG_COLOR")              return COMMAND_BG_COLOR;
    if (command == "CAMERA_POSITION")       return COMMAND_CAMERA_POSITION;
    if (command == "CAMERA_RIGHT")          return COMMAND_CAMERA_RIGHT;
    if (command == "CAMERA_UP")             return COMMAND_CAMERA_UP;
    if (command == "CAMERA_FORWARD")        return COMMAND_CAMERA_FORWARD;
    if (command == "CAMERA_FOV_X")          return COMMAND_CAMERA_FOV_X;
    if (command == "NEW_PRIMITIVE")         return COMMAND_NEW_PRIMITIVE;
    if (command == "PLANE")                 return COMMAND_PLANE;
    if (command == "ELLIPSOID")             return COMMAND_ELLIPSOID;
    if (command == "BOX")                   return COMMAND_BOX;
    if (command == "POSITION")              return COMMAND_POSITION;
    if (command == "ROTATION")              return COMMAND_ROTATION;
    if (command == "COLOR")                 return COMMAND_COLOR;
    if (command == "RAY_DEPTH")             return COMMAND_RAY_DEPTH;
    if (command == "METALLIC")              return COMMAND_METALLIC;
    if (command == "DIELECTRIC")            return COMMAND_DIELECTRIC;
    if (command == "IOR")                   return COMMAND_IOR;
    if (command == "SAMPLES")               return COMMAND_SAMPLES;
    if (command == "EMISSION")              return COMMAND_EMISSION;
    if (command == "TRIANGLE")              return COMMAND_TRIANGLE;

    return -1;
}

std::pair<Primitive, std::string> LoadPrimitive(std::istream &in) {
    Primitive primitive;

    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmd_name;
        ss >> cmd_name;

        auto cmd = get_command(cmd_name);
        if(cmd==COMMAND_EMPTY) break;

        switch (cmd) {
            case COMMAND_ELLIPSOID: {
                Point r;
                ss >> r;
                primitive = Primitive(PRIMITIVE_TYPE::ELLIPSOID, r);
                break;
            }
            case COMMAND_PLANE: {
                Point n;
                ss >> n;
                primitive = Primitive(PRIMITIVE_TYPE::PLANE, n);
                break;
            }
            case COMMAND_BOX: {
                Point s;
                ss >> s;
                primitive = Primitive(PRIMITIVE_TYPE::BOX, s);
                break;
            }
            case COMMAND_TRIANGLE: {
                Point a, b, c;
                ss >> a >> b >> c;
                primitive = Primitive(PRIMITIVE_TYPE::TRIANGLE, a, b, c);
                break;
            }
            case COMMAND_COLOR: {
                ss >> primitive.col;
                break;
            }
            case COMMAND_POSITION: {
                ss >> primitive.pos;
                break;
            }
            case COMMAND_ROTATION: {
                ss >> primitive.rotator;
                break;
            }            
            case COMMAND_METALLIC: {
                primitive.material = Material::METALLIC;
                break;
            }            
            case COMMAND_DIELECTRIC: {
                primitive.material = Material::DIELECTRIC;
                break;
            }            
            case COMMAND_IOR: {
                ss >> primitive.ior;
                break;
            }            
            case COMMAND_EMISSION: {
                ss >> primitive.emission;
                break;
            }            
            
            default: {
                std::cerr << "unexpected primitive(" << cmd_name << ")" << std::endl;
                return std::make_pair(std::move(primitive), cmd_name);
            }
        }
    }
 
    return std::make_pair(std::move(primitive), "");
}

void Scene::Load(std::istream &in) {
    std::string cmds;
    while (getline(in, cmds)) {
        std::stringstream ss;
        ss << cmds;
        std::string cmd_name;
        ss >> cmd_name;

    pasrse_command_again:
        auto cmd = get_command(cmd_name);
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
                auto [primitive, rest_cmd] = LoadPrimitive(in);
                primitives.push_back(std::move(primitive));
                cmd_name = rest_cmd;
                if(!cmd_name.empty()) {
                    goto pasrse_command_again;
                }
                break;
            }
            case COMMAND_RAY_DEPTH: {
                ss >> ray_depth;
                break;
            }
            case COMMAND_SAMPLES: {
                ss >> samples;
                break;
            }
            default: {
                std::cerr << "unexpected command(" << cmd_name << ")" << std::endl;
                break;
            }
        }
    }
}