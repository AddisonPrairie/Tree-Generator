#include "../vec/vec.h"
#include "node.h"

#pragma once

struct TreeSettings {
    int SHADOW_MAP_SIZE_X;
    int SHADOW_MAP_SIZE_Y;
    int SHADOW_MAP_SIZE_Z;
    int SHADOW_Q_MAX;
    float SHADOW_A;
    float SHADOW_B;
    float SHADOW_C;
    float TROPISM_DIR_X;
    float TROPISM_DIR_Y;
    float TROPISM_DIR_Z;
    float TROPISM_ETA;
    float TREE_ETA;
    float LIGHT_ETA;
    float ENERGY_ALPHA;
    float ENERGY_LAMBDA;
    float PRUNE_RATIO;
    float BRANCHING_ANGLE_FACTOR;
    float BRANCH_LENGTH;
    int SHADOW_RADIUS_FACTOR;
};

class Tree {
    public: 
        Tree(TreeSettings settings);
        ~Tree();

        void step();
        void render(class DrawingInfo& drawingInfo);

        void setSettings(TreeSettings settings);

    private:
        float*** _shadowMap = nullptr;
        struct Node* _rootNode = nullptr;

        TreeSettings _settings;

        int _season = 0;

        bool _inShadowMapBounds(int x, int y, int z);

        void _addNodeShadow(Node* node);
        void _removeNodeShadow(Node* node);

        void _deleteNode(Node*& node);

        void _growNode(Node* node);
        void _growNodes();
        vec3f _getOptimalLightDirection(int x, int y, int z);
        void _growShoot(GrowthType type, class Node* node, vec3f offset, float energy);

        float _accumulateLight(Node* node);
        void  _passEnergy(Node* node);

        void _addNodeToRender(Node* node, class DrawingInfo& drawingInfo);
};

struct PointerStack {
    void** _stack = nullptr;
    int _size = 0;
    int _ptr = 0;

    PointerStack(int size) {
        _size = size;
        _stack = new void*[_size];
    }

    void push(void* ptr) {
        if (_ptr >= _size) {
            void** newStack = new void*[(_size *= 2)];

            for (int i = 0; i < _ptr; i++) newStack[i] = _stack[i];

            delete _stack; _stack = newStack;
        }

        _stack[_ptr++] = ptr;
    }

    void* pop() {
        return _stack[--_ptr];
    }

    bool isEmpty() {return _ptr == 0;}
};