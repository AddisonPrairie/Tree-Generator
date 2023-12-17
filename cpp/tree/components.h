
#include "../vec/vec.h"

#pragma once

struct Node {
    struct Node* _pNode = nullptr;
    struct Node* _mNode = nullptr;
    struct Node* _lNode = nullptr;

    vec3f _pos;

    bool _bHasBud = true;

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