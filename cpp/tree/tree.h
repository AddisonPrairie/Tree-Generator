#include "../vec/vec.h"

#pragma once

struct TreeSettings {
    struct {
        int X_SIZE;
        int Y_SIZE;
        int Z_SIZE;
        int Q_MAX;
        float a;
        float b;
    } shadowMap;
};

class Tree {
    public: 
        Tree(TreeSettings settings);
        ~Tree();

        void step();
        void render(class DrawingInfo& drawingInfo);

    private:
        float*** _shadowMap = nullptr;
        struct Node* _rootNode = nullptr;

        TreeSettings _settings;

        int _season = 0;

        bool _inShadowMapBounds(int x, int y, int z);

        void _addNodeShadow(struct Node* node);
        void _removeNodeShadow(struct Node* node);

        Node* _createNode(struct Node* parent, vec3f position);
        void _deleteNode(struct Node*& node);

        void _growNodes();
        void _possiblyPruneNode(struct Node*& node);

        void _addNodeToRender(struct Node* node, class DrawingInfo& drawingInfo);
};