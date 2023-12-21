#include "node.h"

#include <cmath>

Node::Node(GrowthType type, Node* preNode, const vec3f& offset) {
    _preNode = preNode;

    _position = _preNode->_position + offset;

    if (type == AXILLARY) {
        _preNode->_axillaryNode = this;
    }

    if (type == TERMINAL) {
        _preNode->_terminalNode = this;
    }
};

vec3f Node::mainDirection() {
    if (_preNode == nullptr) return {0., 0., 0.};

    return normalize(_position - _preNode->_position);
}

int Node::calculateChildren() {
    if (_bPlaceHolder) return 0;

    int newChildren = 0;

    if (_axillaryNode) {
        newChildren += _axillaryNode->calculateChildren();
    }
    if (_terminalNode) {
        newChildren += _terminalNode->calculateChildren();
    }

    _children = newChildren;
    
    return _children + 1;
}

void Node::calculateRadius() {
    //if this is just a dead place holder branch, do not update the radius
    if (_bPlaceHolder) return;

    if (!_axillaryNode && !_terminalNode) {
        radius = 0.1f;
        return;
    }

    if (_axillaryNode && _terminalNode) {
        _axillaryNode->calculateRadius();
        _terminalNode->calculateRadius();

        float n = 2.5;

        radius = pow(
            pow(_axillaryNode->radius, n) +
            pow(_terminalNode->radius, n),
            1.f / n
        );

        return;
    }

    if (_axillaryNode) {
        _axillaryNode->calculateRadius();

        radius = _axillaryNode->radius;

        return;
    }

    if (_terminalNode) {
        _terminalNode->calculateRadius();

        radius = _terminalNode->radius;

        return;
    }
}