#include <math.h>
#include <cmath>

#include "vec.h"

float _max(float a, float b) {return a > b ? a : b;}
float _min(float a, float b) {return a < b ? a : b;}


//operator overloading <vec3f, vec3f>
vec3f operator+(vec3f a, vec3f b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
};
vec3f operator-(vec3f a, vec3f b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
vec3f operator*(vec3f a, vec3f b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}
vec3f operator/(vec3f a, vec3f b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z};
}

//operator overloading <vec3f, float>
vec3f operator*(vec3f a, float b) {
    return {a.x * b, a.y * b, a.z * b};
}
vec3f operator*(float a, vec3f b) {
    return b * a;
}
vec3f operator/(vec3f a, float b) {
    float inv = 1. / b;
    return {a.x * inv, a.y * inv, a.z * inv};
}
vec3f operator/(float a, vec3f b) {
    return b / a;
}

//<vec3f, ...> -> float functions
float dot(vec3f a, vec3f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
float length(vec3f a) {
    return sqrtf(dot(a, a));
}
float surface_area(vec3f a) {
    return 2.f * (a.x * a.y + a.y * a.z + a.x * a.z);
}
float max(vec3f a) {
    return _max(a.x, _max(a.y, a.z));
}
float min(vec3f a) {
    return _min(a.x, _min(a.y, a.z));
}

//<vec3f, ...> -> vec3f functions
vec3f normalize(vec3f a) {
    return a / length(a);
};
vec3f cross(vec3f a, vec3f b) {
    return 
        {a.y * b.z - a.z * b.y, 
        -a.x * b.z + a.z * b.x, 
         a.x * b.y - a.y * b.x};
}
vec3f abs(vec3f a) {
    return 
        {a.x >= 0.f ? a.x : -a.x,
         a.y >= 0.f ? a.y : -a.y,
         a.z >= 0.f ? a.z : -a.z};
}
vec3f max(vec3f a, vec3f b) {
    return {_max(a.x, b.x), _max(a.y, b.y), _max(a.z, b.z)};
}
vec3f min(vec3f a, vec3f b) {
    return {_min(a.x, b.x), _min(a.y, b.y), _min(a.z, b.z)};
}

//basis/change of basis functions
vec3f ortho(vec3f a) {
    if (abs(a.x) > abs(a.y)) return {-a.y, a.x, 0.f};
    return {0.f, -a.z, a.y};
}
void  basis(vec3f n, vec3f& o1, vec3f& o2) {
    o1 = normalize(ortho(n));
    o2 = normalize(cross(o1, n));
}
vec3f local(vec3f w, vec3f x, vec3f y, vec3f z) {
    return {dot(w, x), dot(w, y), dot(w, z)};
}
vec3f world(vec3f l, vec3f x, vec3f y, vec3f z) {
    return l.x * x + l.y * y + l.z * z;
}