#include "tree.h"
#include "components.h"
#include "../render/drawinginfo.h"

#include <stdio.h>
#include <cmath>

Tree::Tree(TreeSettings settings) {
    _settings = settings;

    //allocate shadow map
    _shadowMap = new float**[_settings.shadowMap.X_SIZE];
    for (int x = 0; x < _settings.shadowMap.X_SIZE; x++) {
        _shadowMap[x] = new float*[_settings.shadowMap.Y_SIZE];
        for (int y = 0; y < _settings.shadowMap.Y_SIZE; y++) {
            _shadowMap[x][y] = new float[_settings.shadowMap.Z_SIZE];
            for (int z = 0; z < _settings.shadowMap.Z_SIZE; z++) {
                _shadowMap[x][y][z] = 0.;//1. - (float) z / (float) _settings.shadowMap.Z_SIZE;
            }
        }
    }

    //create first two nodes
    float midX = (float) _settings.shadowMap.X_SIZE / 2.f;
    float midY = (float) _settings.shadowMap.Y_SIZE / 2.f;

    _rootNode = new Node({midX, midY, 0.f}); _rootNode->_bHasBud = false;
    _rootNode->_mNode = _createNode(_rootNode, {midX, midY, 1.1f});
}

Tree::~Tree() {
    for (int x = 0; x < _settings.shadowMap.X_SIZE; x++) {
        for (int y = 0; y < _settings.shadowMap.Y_SIZE; y++) {
            delete _shadowMap[x][y];
        }
        delete _shadowMap[x];
    }
    delete _shadowMap;
}

void Tree::step() {
    _growNodes();

    if (_season++ % 1 == 0) {
        _possiblyPruneNode(_rootNode);
    }
}

void Tree::render(DrawingInfo& drawingInfo) {
    _addNodeToRender(_rootNode->_mNode, drawingInfo);

    printf("num internodes: %i\n", drawingInfo.getDrawingInfo()[0]);

    return;

    for (int x = 0; x < _settings.shadowMap.X_SIZE; x++) {
        for (int y = 0; y < _settings.shadowMap.Y_SIZE; y++) {
            for (int z = 0; z < _settings.shadowMap.Z_SIZE; z++) {
                if (_shadowMap[x][y][z] > 2.) {
                    vec3f center = vec3f(x + .5f, y + .5f, z + .5f);
                    float r = (_shadowMap[x][y][z] - 2.) / 16.;
                    if (r > .25) r = .25;

                    drawingInfo.addLine(
                        center - vec3f(r,-r, 0.f),
                        vec3f(1., 0., 0.),
                        center + vec3f(r,-r, 0.f),
                        vec3f(1., 0., 0.)
                    );
                    drawingInfo.addLine(
                        center - vec3f(r, r, 0.f),
                        vec3f(1., 0., 0.),
                        center + vec3f(r, r, 0.f),
                        vec3f(1., 0., 0.)
                    );
                    
                }
            }
        }
    }
};

void Tree::_addNodeToRender(Node* node, DrawingInfo& drawingInfo) {
    if (node == nullptr) return;

    drawingInfo.addLine(node->_pNode->_pos, vec3f(1., 1., 1.), node->_pos, vec3f(1., 1., 1.));

    _addNodeToRender(node->_lNode, drawingInfo);
    _addNodeToRender(node->_mNode, drawingInfo);
}

float randf() {
    return (float) rand() / (float) RAND_MAX;
}

void Tree::_growNodes() {
    PointerStack stack(32);

    stack.push(_rootNode);

    while (!stack.isEmpty()) {
        Node* cur = (Node*) stack.pop();

        if (cur->_mNode) stack.push(cur->_mNode);
        if (cur->_lNode) stack.push(cur->_lNode);

        if (cur->_bHasBud == false) continue;

        int x = cur->_pos.x;
        int y = cur->_pos.y;
        int z = cur->_pos.z;

        if (!_inShadowMapBounds(x, y, z)) continue;

        float exposure = 4. - _shadowMap[x][y][z] + _settings.shadowMap.a;

        if (exposure < 1.) {
            cur->_bHasBud = false;
            continue;
        }

        int DIR[26][3] = {
            {-1, -1, -1},
            { 0, -1, -1},
            { 1, -1, -1},
            {-1,  0, -1},
            { 0,  0, -1},
            { 1,  0, -1},
            {-1,  1, -1},
            { 0,  1, -1},
            { 1,  1, -1},

            {-1, -1,  0},
            { 0, -1,  0},
            { 1, -1,  0},
            {-1,  0,  0},
            { 1,  0,  0},
            {-1,  1,  0},
            { 0,  1,  0},
            { 1,  1,  0},

            {-1, -1,  1},
            { 0, -1,  1},
            { 1, -1,  1},
            {-1,  0,  1},
            { 0,  0,  1},
            { 1,  0,  1},
            {-1,  1,  1},
            { 0,  1,  1},
            { 1,  1,  1},
        };

        unsigned long bValid = 0x0;
        
        float least = 1e30f;

        int numValid = 0;
        for (int i = 0; i < 26; i++) {
            const int xx = x + DIR[i][0];
            const int yy = y + DIR[i][1];
            const int zz = z + DIR[i][2];

            if (_inShadowMapBounds(xx, yy, zz)) {
                if (_shadowMap[xx][yy][zz] < least) {
                    least = _shadowMap[xx][yy][zz];
                    bValid = 0x1 << i;
                    numValid = 1;
                } else if (_shadowMap[xx][yy][zz] == least) {
                    bValid |= 0x1 << i;
                    numValid++;
                }
            }
        }

        int rIndex = rand() % numValid;
        for (int i = 0; i < 26; i++) {
            if (bValid & (0x1 << i)) {
                if (rIndex == 0) {
                    rIndex = i;
                    break;
                }
                rIndex--;
            }
        }

        vec3f lightDirection = normalize(
            vec3f((float) DIR[rIndex][0], (float) DIR[rIndex][1], (float) DIR[rIndex][2])
        );

        Node** toSet = nullptr;

        vec3f treeDirection;

        if (!cur->_mNode) {
            toSet = &cur->_mNode;

            treeDirection = normalize(cur->mainDirection());
        } else {
            toSet = &cur->_lNode;

            float r = randf() * 2.f * 3.1415f;
            float r2= randf() * 3.1415f * .5f * .1;

            vec3f localDir = {cos(r) * cos(r2), sin(r) * cos(r2), sin(r2)};

            localDir = normalize(localDir + vec3f(0., 0., 1.) * 1.f);

            vec3f o1, o2;
            basis(normalize(cur->mainDirection()), o1, o2);
            treeDirection = world(localDir, o1, o2, normalize(cur->mainDirection()));

            cur->_bHasBud = false;
        }

        *toSet = _createNode(
            cur,
            cur->_pos + normalize(
                lightDirection * 1. + 
                treeDirection * 10. + 
                vec3f(0., 0., 1.) * -1.f
            )
        );
    }
}

void Tree::_possiblyPruneNode(Node*& node) {
    if (node == nullptr) return;

    if (node->_lNode) {
        _possiblyPruneNode(node->_lNode);
    }
    if (node->_mNode) {
        _possiblyPruneNode(node->_mNode);
    }

    if (node->_bHasBud) return;

    bool bPruneM = node->_mNode == nullptr;
    bool bPruneL = node->_lNode == nullptr;

    if (bPruneL && bPruneM) {
        _deleteNode(node);
    }
}

bool Tree::_inShadowMapBounds(int x, int y, int z) {
    return 
        x >= 0 && x < _settings.shadowMap.X_SIZE &&
        y >= 0 && y < _settings.shadowMap.Y_SIZE &&
        z >= 0 && z < _settings.shadowMap.Z_SIZE;
}

void Tree::_addNodeShadow(Node* node) {
    if (node == nullptr) return;

    for (int q = 0; q < _settings.shadowMap.Q_MAX; q++) {
        for (int i = -q; i < q + 1; i++) {
            for (int j = -q; j < q + 1; j++) {
                int x = node->_pos.x + i;
                int y = node->_pos.y + j;
                int z = node->_pos.z - q;

                if (_inShadowMapBounds(x, y, z)) {
                    _shadowMap[x][y][z] += _settings.shadowMap.a * pow(_settings.shadowMap.b, (float) -q);
                }
            }
        }
    }
}

void Tree::_removeNodeShadow(Node* node) {
    if (node == nullptr) return;

    for (int q = 0; q < _settings.shadowMap.Q_MAX; q++) {
        for (int i = -q; i < q + 1; i++) {
            for (int j = -q; j < q + 1; j++) {
                int x = node->_pos.x + i;
                int y = node->_pos.y + j;
                int z = node->_pos.z - q;

                if (_inShadowMapBounds(x, y, z)) {
                    _shadowMap[x][y][z] -= _settings.shadowMap.a * pow(_settings.shadowMap.b, (float) -q);
                }
            }
        }
    }
}

Node* Tree::_createNode(Node* parent, vec3f position) {
    Node* ret = new Node(parent, position);

    _addNodeShadow(ret);

    return ret;
}

void Tree::_deleteNode(Node*& node) {
    if (node == nullptr) return;

    _removeNodeShadow(node);

    delete node; node = nullptr;
}