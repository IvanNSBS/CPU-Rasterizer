#pragma once

#ifndef SCENEH
#define SCENEH

#include "object.h"
#include "ray.h"
#include <vector>
#include <limits>
#include "Camera.h"

#include <random>

std::default_random_engine gen; 
std::uniform_real_distribution<float> distr(0, 1);
float random_01() { return distr(gen); }

float clamp(const float& a, const float &min, const float &max)
{
    if(a < min)
        return min;
    else if(a > max)
        return max;

    return a;
}

struct SceneOptions
{
    float tmin, tmax; 
    int rayDepth, raysPerPixel;
    SceneOptions(const float &ttmin = 0.001, 
                const float &ttmax = std::numeric_limits<float>::max(),
                const int &maxDepth = 5, const int &rpPixel = 100):tmin(ttmin), tmax(ttmax), rayDepth(maxDepth), raysPerPixel(rpPixel){}
};

class Scene {
public:
    std::vector<Object*> objects;
    std::vector<Object*> lights;
    Camera *camera;
    SceneOptions options;

public:
    
    Scene(): objects(), camera(nullptr){}
    Scene(Camera* cam, const SceneOptions &opt = 0): objects(), camera(cam), options(opt){}

    //Implementa o modelo de iluminacao escolhido, retornando uma cor
    vec3 trace(const Ray& r, int depth){
        if(depth > options.rayDepth)
            return vec3(1,1,1);

        Object* hitted = NULL;
        HitRecord rec;
        // Encontra o ponto intersecao do raio com a esfera que o raio atingiu primeiro.
        // ponto é salvo em HitRecord rec.
        for (unsigned i = 0; i < objects.size(); ++i)
        { 
            //hitrecord temporario. Salva o hitrecord da esfera atual para verificar qual é o mais proximo
            HitRecord temp_hrec; 
            if ( objects[i]->intersect(r, options.tmin, options.tmax, temp_hrec) ) 
            {  
                if (temp_hrec.t < rec.t) 
                { 
                    hitted = objects[i];
                    rec = temp_hrec;
                } 
            } 
        } 

        //Se nao tem intersecao retorna background color
        if (!hitted)
        {
            vec3 unit_direction = r.getDirection();
            float t = 0.5 * (unit_direction.y() + 1.0);
            return (vec3(1.0, 1.0, 1.0) * (1.0 - t) ) + (vec3(0.5, 0.7, 1.0)*t);
        }

        vec3 surfaceColor(0,0,0);
        float bias = 1e-4; //Bias para evitar shadow acne

        switch (hitted->material->matType)
        {
            case mat_type::lambertian:{  
                //Computa o global illumination pelo path tracing
                Ray scattered;
                vec3 attenuation;
                ScatterInfo sinfo;

                //global illumination
                if(hitted->material->scatter(r, rec.phit, rec.nhit, sinfo))
                    surfaceColor = sinfo.attenuation*trace(sinfo.r1, depth+1);
                    // surfaceColor = sinfo.attenuation*trace(sinfo.r1, depth+1);

                if( true ){
                    for (unsigned i = 0; i < lights.size(); ++i) 
                    { 
                        vec3 lightDirection = (lights[i]->center + random_in_unit_sphere()*dynamic_cast<Sphere*>(lights[i])->radius) - rec.phit;  
                        lightDirection.make_unit_vector(); 
                        HitRecord last_rec;
                        last_rec.t = options.tmax;
                        for (unsigned j = 0; j < objects.size(); ++j)
                        { 
                            if (objects[j]->material->matType != mat_type::light && objects[j] != hitted )
                            {
                                Ray n = Ray(rec.phit + rec.nhit * bias, lightDirection);
                                HitRecord lrec;
                                if (objects[j]->intersect(n, options.tmin, options.tmax, lrec) )
                                    if ( lrec.t < last_rec.t )
                                        last_rec = lrec;
                            } 
                        }
                        if ( last_rec.hitted == nullptr ){
                            //Vetor da reflexao da luz com a superficie
                            vec3 R = reflect(lightDirection, rec.nhit);

                            //Fator difuso pelo modelo de iluminacao de Phong
                            vec3 diffuse = hitted->material->surfaceColor * lights[i]->material->emissionColor * lights[i]->material->lightIntensity * 
                            std::max(float(0), dot(rec.nhit, lightDirection));

                            //Fator especular pelo modelo de iluminacao de Phong
                            vec3 specular = lights[i]->material->emissionColor * lights[i]->material->lightIntensity * 
                            pow(std::max(float(0), dot(R, r.getDirection()*-1)), hitted->material->alpha);
                                        
                            //Aplica o modelo de iluminacao de Phong completo na surfaceColor
                            surfaceColor += ((diffuse * hitted->material->Kd) + (specular * hitted->material->Ks));
                        }
                        else
                            surfaceColor *= last_rec.mat->surfaceColor;

                    }
                }
                break;
            }

            case mat_type::conductor:{
                ScatterInfo sinfo;
                if(hitted->material->scatter(r, rec.phit, rec.nhit, sinfo)){
                    surfaceColor = sinfo.attenuation*trace(sinfo.r1, depth+1);
                }
                // surfaceColor = hitted->material->surfaceColor*trace(sinfo.r1, depth+1);
                if( true ){
                    for (unsigned i = 0; i < lights.size(); i++) 
                    { 
                        vec3 transmission(1,1,1); 
                        vec3 lightDirection = (lights[i]->center + random_in_unit_sphere()*dynamic_cast<Sphere*>(lights[i])->radius) - rec.phit;  
                        lightDirection.make_unit_vector(); 
                        for (unsigned j = 0; j < objects.size(); j++)
                        { 
                            if (objects[j]->material->matType != mat_type::light)
                            { 
                                Ray n = Ray(rec.phit + rec.nhit * bias, lightDirection);
                                HitRecord lrec;
                                if (objects[j]->intersect(n, options.tmin, options.tmax, lrec) )
                                {  
                                    transmission = vec3(0,0,0);
                                    break; 
                                } 
                            } 
                        }
                        //Vetor da reflexao da luz com a superficie
                        vec3 R = reflect(lightDirection, rec.nhit);

                        //Fator difuso pelo modelo de iluminacao de Phong
                        vec3 diffuse = 1.0 *   
                        hitted->material->surfaceColor * lights[i]->material->emissionColor * lights[i]->material->lightIntensity * 
                        std::max(float(0), dot(rec.nhit, lightDirection));

                        vec3 specular = 1.0 * lights[i]->material->emissionColor * lights[i]->material->lightIntensity * 
                        pow(std::max(float(0), dot(R, r.getDirection()*-1)), hitted->material->alpha);
                                    
                        //Aplica o modelo de iluminacao de Phong completo na surfaceColor
                        // surfaceColor += ((diffuse * hitted->material->Kd) + (specular * hitted->material->Ks))*transmission;
                    }
                }
                break;
            } 

            case mat_type::dielectric:{
                ScatterInfo sinfo;
                hitted->material->scatter(r, rec.phit, rec.nhit, sinfo);
                float chance = random_01();
                if( chance <= 0.34)
                    surfaceColor = trace(sinfo.r1, depth + 1); 
                else
                    surfaceColor = trace(sinfo.r2, depth + 1);
                // vec3 refractionColor = trace(sinfo.r2, depth + 1);
                // surfaceColor = ((reflectionColor * sinfo.kr) + (refractionColor * (1 - sinfo.kr))) * hitted->material->surfaceColor; 
                break; 
            }

            case mat_type::light :{
                // ScatterInfo sinfo;
                // hitted->material->scatter(r, rec.phit, rec.nhit, sinfo);
                surfaceColor = (hitted->material->emissionColor * hitted->material->lightIntensity);
                break;
            }
        }
        //Delimita o valor da cor entre 0 e 1
        return (surfaceColor);// + (hitted->material->emissionColor * hitted->material->lightIntensity));
        // return vec3( clamp(surfaceColor.x, 0.0, 1.0), clamp(surfaceColor.y, 0.0, 1.0), 
                        //  clamp(surfaceColor.z, 0.0, 1.0) ) + (hitted->material->emissionColor * hitted->material->lightIntensity);
    }


    //Renderiza
    void RenderScene(int minX, int minY, int boundX, int boundY)
    {   
        //Joga raios na cena
        for (unsigned y = minY; y < boundY; ++y)
            for (unsigned x = minX; x < boundX; ++x)
            {
                vec3 col(0,0,0); //cor resultante dos raios

                //Para cada pixel, joga X raios
                //Cada raio possui um offset aleatorio
                //calculando a media dos cem raios acumulados, conseguimos produzir anti-aliasing,
                //além de reduzir o ruido provocado pelo path tracing
                //(quanto mais raios, melhor o AA(super-sampling) e a reduçao de ruido)
                int rayNum = options.raysPerPixel;
                for(int i = 0; i < rayNum; i++){
                    col = col + trace( camera->GetRay( x + random_01(), y + random_01()), 0 );
                }
                float ns = float(rayNum);
                col = vec3 (col.r()/ns, col.g()/ns, col.b()/ns);
                this->camera->canvas->SetPixel( x, y, col );  //seta a cor do pixel para a coordenada
            }
        
        //Salva o ppm como RenderedCanvas na pasta RenderedImages
    }
    
    //void RenderScene();
    //Adiciona um objeto a cena
    void add(Object* object)
    {
        objects.push_back(object);
        if(object->material->matType == mat_type::light)
            lights.push_back(object);
    }
};

#endif 