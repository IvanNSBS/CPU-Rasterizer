#pragma once

#include "../../vec3.h"
#include "float.h"

class Ray
{
private: 
    vec3 origin, direction;

public:
    Ray(): origin( vec3(0,0,0)), direction(vec3(0,0,0)){};
    Ray(vec3 o, vec3 d): origin(o), direction(d){};

    vec3 sample(const double& t) const{
        return this->origin + (this->direction * t);
    }

    vec3 getOrigin() const{
        return origin;
    }

    vec3 getDirection() const{
        return direction;
    }
};