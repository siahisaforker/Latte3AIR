#include "EngineAPI.h"
#include <iostream>

int main() {
    EngineAPI engine;
    engine.initialize();
    std::cout << "Engine initialized. Running main loop..." << std::endl;

    while (true) {
        engine.runFrame();
    }

    return 0;
}
