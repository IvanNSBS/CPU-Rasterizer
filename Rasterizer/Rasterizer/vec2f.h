#ifndef VEC2FH
#define VEC2FH

#include <math.h>
#include <stdlib.h>
#include <iostream>

class vec2f
{
public:
    float e[2];
    
    vec2f(){}
    vec2f(float e0, float e1) { e[0] = e0, e[1] = e1; }
    inline float x() const { return e[0]; }
    inline float y() const { return e[1]; }

    inline const vec2f& operator+() const { return *this; }
    inline vec2f operator-() const { return vec2f(-e[0], -e[1]); }
    inline float operator[](int i) const { return e[i]; }
    inline float& operator[](int i) { return e[i]; }

    inline vec2f& operator +=(const vec2f &v2);
    inline vec2f& operator -=(const vec2f &v2);
    inline vec2f& operator *=(const vec2f &v2);
    inline vec2f& operator /=(const vec2f &v2);
    inline vec2f& operator *=(const float t);
    inline vec2f& operator /=(const float t);

    inline float length() const{ return sqrt(e[0]*e[0] + e[1]*e[1]); }
    inline float squared_length() const{ return e[0]*e[0] + e[1]*e[1]; }
};

inline std::istream& operator>>(std::istream &is, vec2f &t) {
    is >> t.e[0] >> t.e[1];
    return is;
}

inline std::ostream& operator<<(std::ostream &os, const vec2f &t) {
    os << t.e[0] << " " << t.e[1];
    return os;
}

inline vec2f operator+(const vec2f &v1, const vec2f &v2) {
    return vec2f(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1]);
}

inline vec2f operator-(const vec2f &v1, const vec2f &v2) {
    return vec2f(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1]);
}

inline vec2f operator*(const vec2f &v1, const vec2f &v2) {
    return vec2f(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1]);
}

inline vec2f operator/(const vec2f &v1, const vec2f &v2) {
    return vec2f(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1]);
}

inline vec2f operator*(float t, const vec2f &v) {
    return vec2f(t*v.e[0], t*v.e[1]);
}

inline vec2f operator/(vec2f v, float t) {
    return vec2f(v.e[0]/t, v.e[1]/t);
}

inline vec2f operator*(const vec2f &v, float t) {
    return vec2f(t*v.e[0], t*v.e[1]);
}

inline float dot(const vec2f &v1, const vec2f &v2) {
    return v1.e[0] *v2.e[0] + v1.e[1] *v2.e[1];
}

inline vec2f& vec2f::operator+=(const vec2f &v){
    e[0]  += v.e[0];
    e[1]  += v.e[1];
    return *this;
}

inline vec2f& vec2f::operator*=(const vec2f &v){
    e[0]  *= v.e[0];
    e[1]  *= v.e[1];
    return *this;
}

inline vec2f& vec2f::operator/=(const vec2f &v){
    e[0]  /= v.e[0];
    e[1]  /= v.e[1];
    return *this;
}

inline vec2f& vec2f::operator-=(const vec2f& v) {
    e[0]  -= v.e[0];
    e[1]  -= v.e[1];
    return *this;
}

inline vec2f& vec2f::operator*=(const float t) {
    e[0]  *= t;
    e[1]  *= t;
    return *this;
}

inline vec2f& vec2f::operator/=(const float t) {
    float k = 1.0/t;
    e[0]  *= k;
    e[1]  *= k;
    return *this;
}

inline vec2f unit_vector(vec2f v) {
    return v / v.length();
}

#endif