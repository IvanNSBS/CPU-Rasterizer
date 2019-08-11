#pragma once

#include "../../vec3.h"
#include "ray.h"
#include "../../matrix44.h"
#include "image.h"
#include <cmath>

#define M_PI 3.141592653589793

float random()
{
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

vec3 Rand_In_Disk()
{
    vec3 p;
    do{
        
        p = (vec3(random(), random(), 0) * 2) - vec3(1.0, 1.0, 0.0);
    }while( dot(p, p) >= 1.0);

    return p;
}

class Camera
{
public: 
    vec3 position, axisX, axisY, axisZ;
    matrix44 camToWorld;   //matriz para posicionar a camera. Setada no construtor
    Image* canvas;          //Imagem renderizada pela camera. É setada no RenderScene() do Scene.h  
    float fov, near;
    float angle, half_width, half_height;
    float aspectratio;
    float invWidth, invHeight;
public:

    //construtor sem setar Canvas
    Camera (vec3 position, vec3 target, vec3 up, float fov, float near){
        this->position = position;

        this->fov = fov;
        this->near = near;
        
        SetCamAxisAndMatrix(position, target, up);
    }
    //construtor setando canvas
    Camera (Image *im, vec3 position, vec3 target, vec3 up, float fov, float near){
        this->canvas = im;
        this->position = position;

        this->fov = fov;
        this->near = near;

        this->invWidth = 1 / float(canvas->width), invHeight = 1 / float(canvas->height); 
        this->aspectratio = canvas->width / float(canvas->height); 
        //Angulo de abertura da camera
        this->angle = tan((this->fov * 0.5) * (M_PI / 180)) * near ; //Multiplica pelo near (zoom)
        this->half_width = angle * aspectratio;
        this->half_height = angle;
        SetCamAxisAndMatrix(position, target, up);
    }

    //faz a mesma coisa que o construtor antes fazia, 
    //mas como agora existem 2, essa funçao deixa mais organizado as coisas
    void SetCamAxisAndMatrix(vec3 position, vec3 target, vec3 up){
        //Todos os eixos devem ser normalizados
        this->axisZ = unit_vector((position-target));

        //Garante que o vetor up(axisY) eh ortogonal ao eixo z
        //pois a entrada pode ser um vetor nao ortogonal
        //Isso eh feito utilizando o metodo Gram Schmidt 
        this->axisY = up - (axisZ * ( ( dot(axisZ, up) )/ dot(axisZ, axisZ) ) );
        this->axisY.make_unit_vector();

        //Eixo x deve ser ortogonal ao eixo y e ao eixo z
        //Para isso, realiza o produto vetorial entre y e z, que ja sao ortogonais entre si
        this->axisX = cross(axisY, axisZ);
        this->axisX.make_unit_vector();

        //Seta matriz de transformacao afim para ser capaz de movimentar a camera.
        //Parte linear da transformacao
        camToWorld[0][0] = axisX.x(); 
        camToWorld[0][1] = axisX.y(); 
        camToWorld[0][2] = axisX.z(); 

        camToWorld[1][0] = axisY.x(); 
        camToWorld[1][1] = axisY.y(); 
        camToWorld[1][2] = axisY.z(); 

        camToWorld[2][0] = axisZ.x(); 
        camToWorld[2][1] = axisZ.y(); 
        camToWorld[2][2] = axisZ.z(); 
        
        //Parte nao linear (translacao)
        camToWorld[3][0] = position.x(); 
        camToWorld[3][1] = position.y(); 
        camToWorld[3][2] = position.z();
    }

    //GetRay retorna o raio jogado no centro do pixel a partir da camera.
    //Leva em consideração posição da camera e seus eixos
    //Ler comentarios na implementaçao para mais detalhes de como as coisas funcionam
    Ray GetRay(float x, float y)
    {
        //Converte o ponto x do canvas para raster space
        //Multiplica pelo aspectratio, pois o canvas pode nao ser quadrado e gerar distorcao
        float Px = (2 * ( (x ) * invWidth) - 1) * half_width; 
        float Py = (1 - 2 * ( (y ) * invHeight)) * half_height; 

        vec3 origin = vec3(0,0,0);
        vec3 dir = vec3(Px, Py, -1) - origin;
        camToWorld.multVecMatrix(origin, origin);
        camToWorld.multDirMatrix(dir, dir);
        dir.make_unit_vector();
        return Ray(origin , dir);
    }
};