#include "scene.h"

#include <fstream>

int main(int argc, const char *argv[]) {
    (void) argc;

    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);
    
    Scene scene;
    scene.load(in);
    scene.render(out);

    return 0;
}