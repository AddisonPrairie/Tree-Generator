
#include <stdio.h>
#include <cmath>
#include <time.h>

#include "render/drawinginfo.h"
#include "tree/tree.h"

/*

compile with:
    em++ cpp/main.cpp cpp/tree/tree.cpp cpp/vec/vec.cpp -o wasm.js -s EXPORTED_RUNTIME_METHODS=['ccall'] -s WASM=1 -s EXPORTED_FUNCTIONS=['_init','_testMesh'] -s TOTAL_MEMORY=512MB

    em++ cpp/main.cpp cpp/tree/tree.cpp cpp/vec/vec.cpp cpp/tree/node.cpp -o wasm.js -s EXPORTED_RUNTIME_METHODS=['ccall'] -s WASM=1 -s EXPORTED_FUNCTIONS=['_init','_getTreeMesh','_step','_setSettings'] -s ALLOW_MEMORY_GROWTH=1 -O3
*/

extern "C" {

    Tree* tree = nullptr;

    int init(
        int SHADOW_MAP_SIZE_X,
        int SHADOW_MAP_SIZE_Y,
        int SHADOW_MAP_SIZE_Z) {
        printf("hello from cpp\n");

        TreeSettings settings;
        settings.SHADOW_MAP_SIZE_X = SHADOW_MAP_SIZE_X;
        settings.SHADOW_MAP_SIZE_Y = SHADOW_MAP_SIZE_Y;
        settings.SHADOW_MAP_SIZE_Z = SHADOW_MAP_SIZE_Z;

        tree = new Tree(settings);

        srand(time(NULL));

        return 0;
    }

    void setSettings(
        int SHADOW_Q_MAX,
        float SHADOW_A,
        float SHADOW_B,
        float SHADOW_C,
        float TROPISM_DIR_X,
        float TROPISM_DIR_Y,
        float TROPISM_DIR_Z,
        float TROPISM_ETA,
        float TREE_ETA,
        float LIGHT_ETA,
        float ENERGY_ALPHA,
        float ENERGY_LAMBDA,
        float PRUNE_RATIO,
        float BRANCHING_ANGLE_FACTOR,
        float BRANCH_LENGTH,
        int SHADOW_RADIUS_FACTOR
    ) {
        TreeSettings settings;

        settings.SHADOW_Q_MAX = SHADOW_Q_MAX;
        settings.SHADOW_A = SHADOW_A;
        settings.SHADOW_B = SHADOW_B;
        settings.SHADOW_C = SHADOW_C;
        settings.TROPISM_DIR_X = TROPISM_DIR_X;
        settings.TROPISM_DIR_Y = TROPISM_DIR_Y;
        settings.TROPISM_DIR_Z = TROPISM_DIR_Z;
        settings.TROPISM_ETA   = TROPISM_ETA;
        settings.TREE_ETA = TREE_ETA;
        settings.LIGHT_ETA = LIGHT_ETA;
        settings.ENERGY_ALPHA = ENERGY_ALPHA;
        settings.ENERGY_LAMBDA = ENERGY_LAMBDA;
        settings.PRUNE_RATIO = PRUNE_RATIO;
        settings.BRANCHING_ANGLE_FACTOR = BRANCHING_ANGLE_FACTOR;
        settings.BRANCH_LENGTH = BRANCH_LENGTH;
        settings.SHADOW_RADIUS_FACTOR = SHADOW_RADIUS_FACTOR;

        if (tree) {
            tree->setSettings(settings);
        }
    }

    void step() {
        tree->step();
    }

    DrawingInfo* info = nullptr;

    int* getTreeMesh() {
        if (info) {
            delete info;
        }

        info = new DrawingInfo();

        tree->render(*info);

        return info->getDrawingInfo();
    }
}