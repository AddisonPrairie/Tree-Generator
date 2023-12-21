#include "tree.h"
#include "node.h"
#include "../render/drawinginfo.h"

#include <stdio.h>
#include <cmath>

Tree::Tree(TreeSettings settings) {
    _settings = settings;

    //allocate shadow map
    _shadowMap = new float**[_settings.SHADOW_MAP_SIZE_X];
    for (int x = 0; x < _settings.SHADOW_MAP_SIZE_X; x++) {
        _shadowMap[x] = new float*[_settings.SHADOW_MAP_SIZE_Y];
        for (int y = 0; y < _settings.SHADOW_MAP_SIZE_Y; y++) {
            _shadowMap[x][y] = new float[_settings.SHADOW_MAP_SIZE_Z];
            for (int z = 0; z < _settings.SHADOW_MAP_SIZE_Z; z++) {
                _shadowMap[x][y][z] = 0.;
            }
        }
    }

    //create first two nodes
    float midX = (float) _settings.SHADOW_MAP_SIZE_X / 2.f;
    float midY = (float) _settings.SHADOW_MAP_SIZE_Y / 2.f;

    _rootNode = new Node();
    _rootNode->_position = vec3f(midX, midY, 0.f);
    _rootNode->_bHasBud = false;
    
    new Node(TERMINAL, _rootNode, vec3f(0.f, 0.f, 1.1f));
}

void Tree::setSettings(TreeSettings settings) {
    //does not let the user change the size of the shadow map
    settings.SHADOW_MAP_SIZE_X = _settings.SHADOW_MAP_SIZE_X;
    settings.SHADOW_MAP_SIZE_Y = _settings.SHADOW_MAP_SIZE_Y;
    settings.SHADOW_MAP_SIZE_Z = _settings.SHADOW_MAP_SIZE_Z;

    _settings = settings;
}

Tree::~Tree() {
    for (int x = 0; x < _settings.SHADOW_MAP_SIZE_X; x++) {
        for (int y = 0; y < _settings.SHADOW_MAP_SIZE_Y; y++) {
            delete _shadowMap[x][y];
        }
        delete _shadowMap[x];
    }
    delete _shadowMap;
}

void Tree::step() {
    if (!_rootNode->_terminalNode) return;

    _rootNode->_terminalEnergy = 1.f * _accumulateLight(_rootNode->_terminalNode);

    _passEnergy(_rootNode->_terminalNode);

    _rootNode->calculateChildren();

    return;
}

void Tree::render(DrawingInfo& drawingInfo) {
    PointerStack stack(512);

    _rootNode->_terminalNode->calculateRadius();

    stack.push(_rootNode->_terminalNode);

    while (!stack.isEmpty()) {
        Node* current = (Node*) stack.pop();

        if (current == nullptr) continue;

        if (current->_axillaryNode) stack.push(current->_axillaryNode);
        if (current->_terminalNode) stack.push(current->_terminalNode);

        _addNodeToRender(current, drawingInfo);
    }

    drawingInfo.addBoundingBox(
        vec3f(0.f),
        vec3f(
            _settings.SHADOW_MAP_SIZE_X,
            _settings.SHADOW_MAP_SIZE_Y,
            _settings.SHADOW_MAP_SIZE_Z),
        vec3f(64.f / 255.f)
    );

    for (int i = 10; i < _settings.SHADOW_MAP_SIZE_X; i += 10) {
        drawingInfo.addLine(
            vec3f(i, 0.f, 0.f),
            vec3f(64.f / 255.f),
            vec3f(i, _settings.SHADOW_MAP_SIZE_Y, 0.f),
            vec3f(64.f / 255.f)
        );
    }

    for (int i = 0; i < _settings.SHADOW_MAP_SIZE_Y; i += 10) {
        drawingInfo.addLine(
            vec3f(0.f, i, 0.f),
            vec3f(64.f / 255.f),
            vec3f(_settings.SHADOW_MAP_SIZE_X, i, 0.f),
            vec3f(64.f / 255.f)
        );
    }

    return;
    
    for (int x = 0; x < _settings.SHADOW_MAP_SIZE_X; x++) {
        for (int y = 0; y < _settings.SHADOW_MAP_SIZE_Y; y++) {
            for (int z = 0; z < _settings.SHADOW_MAP_SIZE_Z; z++) {
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
    {
        float c0 = node->_preNode->_bHasBud ? 1.f : 0.f;
        float c1 = node->_bHasBud ? 1.f : 0.f;

        drawingInfo.addLine(
            node->_preNode->_position,
            //vec3f(58.f / 255.f, 95.f / 255.f, 11.f / 255.f) * 2.f * c0,
            vec3f(0.f),
            node->_position,
            //vec3f(58.f / 255.f, 95.f / 255.f, 11.f / 255.f) * 2.f * c1
            vec3f(0.f)
        );
    }
    {
        vec3f up;

        if (node->_terminalNode) {
            up = normalize((node->_terminalNode->mainDirection() + node->mainDirection()));
        } else {
            up = node->mainDirection();
        }

        vec3f o1, o2;

        basis(up, o1, o2);

        const int numPoints = 6;

        float lastR = (float) (numPoints - 1) / (float) numPoints * 2.f * 3.1415f;

        float radius = node->radius;

        if (radius < .2f) return;

        vec3f lastPos = node->_position + world(vec3f(cos(lastR) * radius, sin(lastR) * radius, 0.f), o1, o2, up);

        for (int i = 0; i < numPoints; i++) {
            float r = (float) i / (float) numPoints * 2.f * 3.1415f;

            vec3f lPos = {cos(r) * radius, sin(r) * radius, 0.};

            vec3f wPos = node->_position + world(lPos, o1, o2, up);

            drawingInfo.addLine(
                lastPos, //vec3f(.5),
                vec3f(.8f),//
                //vec3f(150.f / 255.f, 75.f / 255.f, 0.f) * .5,
                wPos, //vec3f(.5)
                vec3f(.8f)
                //vec3f(150.f / 255.f, 75.f / 255.f, 0.f) * .5
            );

            lastPos = wPos;
        }  
    }
}

float Tree::_accumulateLight(Node* node) {
    if (node == nullptr) return 0.f;

    node->_axillaryEnergy = 0.f;
    node->_terminalEnergy = 0.f;

    if (node->_terminalNode) {
        node->_terminalEnergy = _accumulateLight(node->_terminalNode);

        float energyPerNode = node->_terminalEnergy / (float) node->_terminalNode->_children;

        if (energyPerNode < _settings.PRUNE_RATIO) {
            _deleteNode(node->_terminalNode);
        }
    }
    if (node->_axillaryNode) {
        node->_axillaryEnergy = _accumulateLight(node->_axillaryNode);

        float energyPerNode = node->_axillaryEnergy / (float) node->_axillaryNode->_children;

        if (energyPerNode < _settings.PRUNE_RATIO) {
            _deleteNode(node->_axillaryNode);
        }
    }

    if (node->_bHasBud) {
        if (!_inShadowMapBounds(node->_position.x, node->_position.y, node->_position.z)) {
            return 0.f;
        };

        float light = _settings.SHADOW_C - _shadowMap[(int)node->_position.x][(int)node->_position.y][(int)node->_position.z] + _settings.SHADOW_A;

        if (light < 0.f) light = 0.f;

        if (!node->_terminalNode) {
            node->_terminalEnergy += light;
        } else
        if (!node->_axillaryNode) {
            node->_axillaryEnergy += light;
        }
    }

    return node->_axillaryEnergy + node->_terminalEnergy;
}

void Tree::_passEnergy(Node* node) {
    if (node == nullptr) return;

    const float lambda = _settings.ENERGY_LAMBDA;

    const float axillaryCoefficient = node->_axillaryEnergy * (1.f - lambda);
    const float terminalCoefficient = node->_terminalEnergy * lambda;

    const float denominator = 1.f / (axillaryCoefficient + terminalCoefficient);

    float incomingEnergy = 0.f;

    //see from which source this branch will be receiving its energy
    if (node->_preNode->_axillaryNode == node) {
        incomingEnergy = node->_preNode->_axillaryEnergy;
    } else {
        incomingEnergy = node->_preNode->_terminalEnergy;
    }

    node->_axillaryEnergy = axillaryCoefficient * incomingEnergy * denominator;
    node->_terminalEnergy = terminalCoefficient * incomingEnergy * denominator;

    if (axillaryCoefficient == terminalCoefficient && axillaryCoefficient == 0) {
        node->_axillaryEnergy = 0.f;
        node->_terminalEnergy = 0.f;
        
        return;
    }

    if (node->_terminalNode) {
        _passEnergy(node->_terminalNode);
    }

    if (node->_axillaryNode) {
        _passEnergy(node->_axillaryNode);
    }

    _growNode(node);
}

float randf() {
    return (float) rand() / (float) RAND_MAX;
}

void Tree::_growNode(Node* node) {
    if (node == nullptr) return;

    //if this node does not have a bud, it cannot grow anymore
    if (!node->_bHasBud) return;

    int x = node->_position.x;
    int y = node->_position.y;
    int z = node->_position.z;

    if (!_inShadowMapBounds(x, y, z)) return;

    GrowthType type;

    if (node->_terminalNode) {
        type = AXILLARY;
    } else {
        type = TERMINAL;
    }

    float energy = type == AXILLARY ? node->_axillaryEnergy : node->_terminalEnergy;

    if (energy < 1.f) {
        if (energy < .1f) {
            node->_bHasBud = false;
        }
        return;
    }

    vec3f treeDirection;

    const float branchingAngle = _settings.BRANCHING_ANGLE_FACTOR;

    //generate a random sample in the disc about the branch main direction,
    //then shift it towards the forward direction of the branch
    if (type == AXILLARY) {
        float r = randf() * 2.f * 3.1415f;

        vec3f localDir = {cos(r), sin(r), 0.};

        localDir = normalize(localDir + vec3f(0., 0., branchingAngle));

        vec3f up = node->mainDirection();

        vec3f o1, o2;
        basis(up, o1, o2);
        treeDirection = world(localDir, o1, o2, up);

        node->_bHasBud = false;
    } 
    //otherwise, the tree just continues growing in the same direction
    else {
        treeDirection = node->mainDirection();
    }

    _growShoot(type, node, treeDirection, energy);
}

//based on the energy a bud receives, grow a number of new nodes in the designated direction
void Tree::_growShoot(GrowthType type, Node* node, vec3f direction, float energy) {
    
    if (energy > 30.f) {
        energy = 30.f; //NOTE: come back to this... not sure how a single bud can get so much
    }

    int numNodes = (int) energy;

    float nodeLength = energy / (float) numNodes;

    const float lightEta = _settings.LIGHT_ETA;
    const float tropismEta = _settings.TROPISM_ETA;
    const vec3f tropismDirection = 
        vec3f(
            _settings.TROPISM_DIR_X, _settings.TROPISM_DIR_Y, _settings.TROPISM_DIR_Z
        );
    const float treeEta = _settings.TREE_ETA;

    for (int i = 0; i < numNodes; i++) {
        direction = normalize(
                direction * treeEta + 
                _getOptimalLightDirection(
                    node->_position.x, 
                    node->_position.y, 
                    node->_position.z) * lightEta + 
                tropismDirection * tropismEta) * nodeLength * _settings.BRANCH_LENGTH;
        
        if (!_inShadowMapBounds(
            node->_position.x + (int) direction.x,
            node->_position.y + (int) direction.y,
            node->_position.z + (int) direction.z)) break;

        node = new Node(type, node, direction);

        _addNodeShadow(node);

        type = TERMINAL;
    }
}

//possible directions look up table for determining optimal direction
//to grow towards
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

vec3f Tree::_getOptimalLightDirection(int x, int y, int z) {
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

    return normalize(
        vec3f((float) DIR[rIndex][0], (float) DIR[rIndex][1], (float) DIR[rIndex][2])
    );
}

bool Tree::_inShadowMapBounds(int x, int y, int z) {
    return 
        x >= 0 && x < _settings.SHADOW_MAP_SIZE_X &&
        y >= 0 && y < _settings.SHADOW_MAP_SIZE_Y &&
        z >= 0 && z < _settings.SHADOW_MAP_SIZE_Z;
}

void Tree::_addNodeShadow(Node* node) {
    if (node == nullptr) return;

    for (int q = 0; q < _settings.SHADOW_Q_MAX; q++) {

        int r = q / _settings.SHADOW_RADIUS_FACTOR;

        for (int i = -r; i < r + 1; i++) {
            for (int j = -r; j < r + 1; j++) {
                int x = node->_position.x + i;
                int y = node->_position.y + j;
                int z = node->_position.z - q;

                float amt = _settings.SHADOW_A * pow(_settings.SHADOW_B, (float) -q);

                if (amt < .01f) goto endloop;

                if (_inShadowMapBounds(x, y, z)) {
                    _shadowMap[x][y][z] += amt;
                }
            }
        }
    }

endloop:
    return;
}

void Tree::_removeNodeShadow(Node* node) {
    if (node == nullptr) return;

    for (int q = 0; q < _settings.SHADOW_Q_MAX; q++) {

        int r = q / _settings.SHADOW_RADIUS_FACTOR;

        for (int i = -r; i < r + 1; i++) {
            for (int j = -r; j < r + 1; j++) {
                int x = node->_position.x + i;
                int y = node->_position.y + j;
                int z = node->_position.z - q;

                float amt = _settings.SHADOW_A * pow(_settings.SHADOW_B, (float) -q);

                if (amt < .01f) goto endloop;

                if (_inShadowMapBounds(x, y, z)) {
                    _shadowMap[x][y][z] -= amt;
                }
            }
        }
    }

endloop:
    return;
}

void Tree::_deleteNode(Node*& node) {
    if (node == nullptr) return;

    _deleteNode(node->_axillaryNode);
    _deleteNode(node->_terminalNode);

    _removeNodeShadow(node);

    delete node; node = nullptr;
}