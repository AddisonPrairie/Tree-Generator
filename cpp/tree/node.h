
#include "../vec/vec.h"

#pragma once

enum GrowthType {
    AXILLARY,
    TERMINAL
};

class Node {
public:
    vec3f _position;

    Node* _axillaryNode = nullptr;
    Node* _terminalNode = nullptr;

    Node* _preNode = nullptr;

    float _axillaryEnergy = 0.f;
    float _terminalEnergy = 0.f;

    bool _bHasBud = true;

    //_children initialy negative to signal a need to recalculate
    int _children = -1;

    float radius = 0.f;

    Node(GrowthType type, Node* preNode, const vec3f& offset);

    Node() {};

    int calculateChildren();

    vec3f mainDirection();

    void calculateRadius();
};