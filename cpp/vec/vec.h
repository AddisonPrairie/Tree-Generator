#pragma once

struct vec3f {
    float x;
    float y;
    float z;

    vec3f(float _x, float _y, float _z) {x = _x; y = _y; z = _z;}
    vec3f(float _a) {x = _a; y = _a; z = _a;}
    vec3f() {}

    float operator[](int index) {
        if (index == 0) return x;
        if (index == 1) return y;
        return z;
    }
};

//operator overloading <vec3f, vec3f>
vec3f operator+(vec3f a, vec3f b);
vec3f operator-(vec3f a, vec3f b);
vec3f operator*(vec3f a, vec3f b);
vec3f operator/(vec3f a, vec3f b);

//operator overloading <vec3f, float>
vec3f operator*(vec3f a, float b);
vec3f operator*(float a, vec3f b);
vec3f operator/(vec3f a, float b);
vec3f operator/(float a, vec3f b);

//<vec3f, ...> -> float functions
float dot(vec3f a, vec3f b);
float length(vec3f a);
float surface_area(vec3f a);
float max(vec3f a);
float min(vec3f a);

//<vec3f, ...> -> vec3f functions
vec3f normalize(vec3f a);
vec3f cross(vec3f a, vec3f b);
vec3f abs(vec3f a);
vec3f max(vec3f a, vec3f b);
vec3f min(vec3f a, vec3f b);

//basis/change of basis functions
vec3f ortho(vec3f a);
void  basis(vec3f n, vec3f& o1, vec3f& o2);
vec3f local(vec3f w, vec3f x, vec3f y, vec3f z);
vec3f world(vec3f l, vec3f x, vec3f y, vec3f z);