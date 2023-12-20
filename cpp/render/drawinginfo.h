#pragma once

#include "../vec/vec.h"

#include <stdio.h>

class DrawingInfo {
    public: 
        DrawingInfo() {
            _capacity = 32;
            _lines = 0;

            //initialize buffer
            _growBuffer();
        };
        ~DrawingInfo() {
            delete _root;
        };

        void addLine(
            const vec3f& p0, 
            const vec3f& c0, 
            const vec3f& p1,
            const vec3f& c1
            ) {
                if (_lines >= _capacity) {
                    _capacity *= 2; _growBuffer();
                }

                int idx = (_lines++) * 12;
                _root[0] = _lines;

                _attributes[idx +  0] = p0.x;
                _attributes[idx +  1] = p0.y;
                _attributes[idx +  2] = p0.z;
                _attributes[idx +  3] = c0.x;
                _attributes[idx +  4] = c0.y;
                _attributes[idx +  5] = c0.z;
                _attributes[idx +  6] = p1.x;
                _attributes[idx +  7] = p1.y;
                _attributes[idx +  8] = p1.z;
                _attributes[idx +  9] = c1.x;
                _attributes[idx + 10] = c1.y;
                _attributes[idx + 11] = c1.z;
            }
        
        int* getDrawingInfo() {
            return _root;
        }

        void addBoundingBox(
            const vec3f& min,
            const vec3f& max,
            const vec3f& col
        ) {
            addLine(
                vec3f(min.x, min.y, min.z), col,
                vec3f(max.x, min.y, min.z), col
            );
            addLine(
                vec3f(min.x, min.y, min.z), col,
                vec3f(min.x, min.y, max.z), col
            );
            addLine(
                vec3f(max.x, min.y, max.z), col,
                vec3f(min.x, min.y, max.z), col
            );
            addLine(
                vec3f(max.x, min.y, min.z), col,
                vec3f(max.x, min.y, max.z), col
            );

            addLine(
                vec3f(min.x, max.y, min.z), col,
                vec3f(max.x, max.y, min.z), col
            );
            addLine(
                vec3f(min.x, max.y, min.z), col,
                vec3f(min.x, max.y, max.z), col
            );
            addLine(
                vec3f(max.x, max.y, max.z), col,
                vec3f(min.x, max.y, max.z), col
            );
            addLine(
                vec3f(max.x, max.y, min.z), col,
                vec3f(max.x, max.y, max.z), col
            );

            addLine(
                vec3f(min.x, min.y, min.z), col,
                vec3f(min.x, max.y, min.z), col
            );
            addLine(
                vec3f(max.x, min.y, min.z), col,
                vec3f(max.x, max.y, min.z), col
            );
            addLine(
                vec3f(max.x, min.y, max.z), col,
                vec3f(max.x, max.y, max.z), col
            );
            addLine(
                vec3f(min.x, min.y, max.z), col,
                vec3f(min.x, max.y, max.z), col
            );
        };

    private:
        int _capacity;
        int _lines;

        float* _attributes = nullptr;
        int* _root = nullptr;

        //grows the underlying buffer to be the size of capacity
        void _growBuffer() {
            int* newRoot = new int[_capacity * 12 + 1];
            newRoot[0] = (_root == nullptr) ? 0 : _root[0];

            float* newAttributes = (float*) &newRoot[1];

            if (_attributes) {
                for (int i = 0; i < _lines * 12; i++) {
                    newAttributes[i] = _attributes[i];
                }
            }

            delete _root;
            _attributes = newAttributes; _root = newRoot;
        }
};