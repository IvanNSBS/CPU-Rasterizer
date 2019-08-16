#ifndef CAMERAH
#define CAMERAH

#include "vec3.h"
#include "vec2f.h"
#include "matrix44.h"


class camera
{
public:
    int imgWidth, imgHeight;
    float fov, _near;
    float bottom, left, top, right;
    matrix44 camToWorld;
    matrix44 worldToCamera;

	vec3 _from, _at, _up;
    vec3 axisX, axisY, axisZ;

    vec3 rotation = vec3(0,0,0);

public:
    camera();
    camera(const vec3 &from, const vec3 &at, const vec3 &up,
           const float &f, const float &n,
           const int &iwidth, const int &iheight): 
           fov(f), _near(n), imgWidth(iwidth), imgHeight(iheight),
		   _from(from), _at(at), _up(up)
           {
                float aspectratio = iwidth/iheight;
                float angle = std::tan((f*0.5f)*3.14f/180.0f);
                top = angle; 
                right = angle * aspectratio;    
                bottom = -top; 
                left = -right;
                set_axis_and_matrix(from, at, up, true);
           }

    void set_axis_and_matrix(const vec3& from, const vec3& at, const vec3& up, bool update_axis = false)
    {
        if( update_axis ){
            axisZ = unit_vector( from-at );
            axisY = unit_vector( up - ( axisZ * ( dot(up,axisZ)/dot(axisZ, axisZ) ) ) );
            axisX = unit_vector( cross(axisY, axisZ) );
        }
        else{
            axisX.make_unit_vector();
            axisY.make_unit_vector();
            axisZ.make_unit_vector();
        }

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

	void move(vec3 dir) {
        camToWorld.multDirMatrix(dir, dir);
		_from += dir;
		_at += dir;
		set_axis_and_matrix( _from, _at, _up, false);
	}

    bool compute_pixel_coordinates(const vec3 &pWorld, vec2f &pRaster) 
    { 
		vec3 ray = pWorld - _from;
		ray.make_unit_vector();
		vec3 lookdir = axisZ;
		if ( dot(ray, lookdir) > 0.0f )
			return false;

        vec3 pCamera; 
        worldToCamera.multVecMatrix(pWorld, pCamera); 
        vec2f pScreen; 
        pScreen[0] = pCamera.x() * _near / (-pCamera.z()); 
        pScreen[1] = pCamera.y() * _near / (-pCamera.z()); 
    
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

    void rot_x(float deg) {

        float sen = sin(deg*3.14 / 180.0f);
        float co = cos(deg*3.14 / 180.0f);
        matrix44 rot(1, 0, 0, 0,
                    0, co, -sen, 0,
                    0, sen, co, 0,
                    0, 0, 0, 1);

        matrix44 tr(		1,			  0,			0,			0,
                            0,			  1,			0,			0,
                            0,			  0,			1,			0,
                    -_from.x(), -_from.y(), -_from.z(), 1);
        matrix44 itr = tr.inverse();
        matrix44 result = (tr*rot)*itr;
        result.multVecMatrix(_at,_at);
        result.multDirMatrix(_up,_up);
        _up.make_unit_vector();

		set_axis_and_matrix( _from, _at, _up, true);


    }

    void rot_y(float deg) {

        float sen = sin(deg*3.14 / 180.0f);
        float co = cos(deg*3.14 / 180.0f);
        matrix44 rot(co, 0, sen, 0,
                    0, 1, 0, 0,
                    -sen, 0, co, 0,
                    0, 0, 0, 1);

        matrix44 tr(		1,			  0,			0,			0,
                            0,			  1,			0,			0,
                            0,			  0,			1,			0,
                    -_from.x(), -_from.y(), -_from.z(), 1);
        matrix44 itr = tr.inverse();

        matrix44 result = (tr*rot)*itr;
        result.multVecMatrix(_at,_at);
        result.multDirMatrix(_up,_up);
        _up.make_unit_vector();

		set_axis_and_matrix( _from, _at, _up, true);

    }

    void rot_z(float deg) {

        matrix44 tr(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
            -_from.x(), -_from.y(), -_from.z(), 1);
        matrix44 itr = tr.inverse();
        float sen = sin(deg*3.14 / 180.0);
        float co = cos(deg*3.14 / 180);
        matrix44 rot(co, -sen, 0, 0,
                     sen, co, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1);
        matrix44 result = (tr*rot)*itr;
        result.multVecMatrix(_at,_at);
        result.multDirMatrix(_up,_up);
        set_axis_and_matrix( _from, _at, _up, true);
        rotation[2] += deg;
    }
};


#endif