#ifndef CAMERAH
#define CAMERAH

#include "vec3.h"
#include "vec2f.h"
#include "matrix44.h"

#define M_PI 3.141592653589793

class camera
{
public:
    int imgWidth, imgHeight;
    float fov, near;
    float bottom, left, top, right;
    matrix44 camToWorld;
    matrix44 worldToCamera;

    vec3 axisX, axisY, axisZ;
public:
    camera();
    camera(const vec3 &from, const vec3 &at, const vec3 &up,
           const float &f, const float &n,
           const int &iwidth, const int &iheight): 
           fov(f), near(n), imgWidth(iwidth), imgHeight(iheight)
           {
                float aspectratio = iwidth/iheight;
                float angle = std::tan((f*0.5)*M_PI/180) * near;
                top = angle; 
                right = angle * aspectratio;    
                bottom = -top; 
                left = -right;
                set_axis_and_matrix(from, at, up);
           }

    void set_axis_and_matrix(const vec3& from, const vec3& at, const vec3& up)
    {
        axisZ = unit_vector( from-at );
        axisY = unit_vector( up - ( axisZ * ( dot(up,axisZ)/dot(axisZ, axisZ) ) ) );
        axisX = unit_vector( cross(axisY, axisZ) );

        camToWorld.x[0][0] = axisX.x(); 
        camToWorld.x[0][1] = axisX.y(); 
        camToWorld.x[0][2] = axisX.z(); 

        camToWorld.x[1][0] = axisY.x(); 
        camToWorld.x[1][1] = axisY.y(); 
        camToWorld.x[1][2] = axisY.z(); 

        camToWorld.x[2][0] = axisZ.x(); 
        camToWorld.x[2][1] = axisZ.y(); 
        camToWorld.x[2][2] = axisZ.z(); 
        
        camToWorld.x[3][0] = from.x(); 
        camToWorld.x[3][1] = from.y(); 
        camToWorld.x[3][2] = from.z();

        worldToCamera = camToWorld.inverse();
    }

    bool compute_pixel_coordinates(const vec3 &pWorld, vec2f &pRaster) 
    { 
        vec3 pCamera; 
        worldToCamera.multVecMatrix(pWorld, pCamera); 
        vec2f pScreen; 
        pScreen[0] = pCamera.x() / (-pCamera.z() * near); 
        pScreen[1] = pCamera.y() / (-pCamera.z() * near); 
    
        vec2f pNDC; 
        pNDC[0] = (pScreen.x() + right) / (2 * right); 
        pNDC[1] = (pScreen.y() + top) / (2 * top); 
        pRaster[0] = (pNDC.x() * imgWidth); 
        pRaster[1] = ((1 - pNDC.y()) * imgHeight); 
    
        bool visible = true; 
        /*if (pScreen.x() < left || pScreen.x() > right || pScreen.y() < bottom || pScreen.y() > top) 
            visible = false;*/ 
    
        return visible; 
    }
};


#endif