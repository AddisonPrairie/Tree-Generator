
#include <stdio.h>
#include <cmath>
#include <time.h>

#include "render/drawinginfo.h"
#include "tree/tree.h"

/*

compile with:
    em++ cpp/main.cpp cpp/tree/tree.cpp cpp/vec/vec.cpp -o wasm.js -s EXPORTED_RUNTIME_METHODS=['ccall'] -s WASM=1 -s EXPORTED_FUNCTIONS=['_init','_testMesh'] -s TOTAL_MEMORY=512MB

    em++ cpp/main.cpp cpp/tree/tree.cpp cpp/vec/vec.cpp -o wasm.js -s EXPORTED_RUNTIME_METHODS=['ccall'] -s WASM=1 -s EXPORTED_FUNCTIONS=['_init','_getTreeMesh','_step'] -s ALLOW_MEMORY_GROWTH=1 -O3
*/

extern "C" {

    Tree* tree = nullptr;

    int init() {
        printf("hello from cpp\n");

        TreeSettings settings;
        settings.shadowMap.X_SIZE = 400;
        settings.shadowMap.Y_SIZE = 400;
        settings.shadowMap.Z_SIZE = 400;

        settings.shadowMap.Q_MAX = 8;
        settings.shadowMap.a = 1.5f;
        settings.shadowMap.b = 1.1f;

        tree = new Tree(settings);

        srand(time(NULL));

        return 0;
    }

    void step() {
        tree->step();
    }

    int* getTreeMesh() {
        DrawingInfo info{};

        tree->render(info);

        return info.getDrawingInfo();
    }
}