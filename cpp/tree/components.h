
#include "../vec/vec.h"

#pragma once

struct Node {
    struct Node* _pNode = nullptr;
    struct Node* _mNode = nullptr;
    struct Node* _lNode = nullptr;

    vec3f _pos;

    bool _bHasBud = true;

    float _lEnergy = 0.;
    float _mEnergy = 0.;

    Node(struct Node* p, vec3f pos) {
        _pNode = p; _pos = pos;
    }

    Node(vec3f pos) {
        _pos = pos;
    }

    vec3f mainDirection() {
        if (_pNode == nullptr) return {0., 0., 0.};

        return normalize(_pos - _pNode->_pos);
    }
};