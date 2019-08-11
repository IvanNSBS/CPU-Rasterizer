#ifndef HITRECORDH
#define HITRECORDH

#include "../../vec3.h"
#include <limits>

class Material;
class Object;

struct HitRecord
{
    vec3 nhit, phit;
    float t = std::numeric_limits<float>::max();
    Material* mat;
    Object* hitted = nullptr; 
};

#endif