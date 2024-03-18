#include "scene.h"

#include <fstream>
#include <iostream>

int main(int argc, const char *argv[]) {
    (void) argc;

    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);

    Scene scene;
    scene.Load(in);
    //scene.LoadDistribution();
    //scene.Render(out);

    return 0;
}