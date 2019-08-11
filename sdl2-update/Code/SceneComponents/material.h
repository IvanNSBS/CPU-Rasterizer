#ifndef MATERIALH
#define MATERIALH

#include "../../vec3.h"
#include "ray.h"
#include <iostream>
#include <random>
#include <string>
#include <algorithm>
#include <utility>
#include <cmath>

vec3 random_in_unit_sphere();

// float drand48();

enum mat_type{ lambertian, conductor, dielectric, light };

//Calcula o vetor apos a reflexao com a superficie
vec3 reflect( vec3 &v, const vec3& n) {
     return v - (n * (2*dot(v, n)));
}

float limit(const float& a, const float &min, const float &max)
{
    if(a < min)
        return min;
    else if(a > max)
        return max;

    return a;
}

void fresnel(vec3 &I, const vec3 &N, const float &ior, float &kr) 
{ 
    float cosi = limit(dot(I,N),-1, 1); 
    float etai = 1, etat = ior; 
    if (cosi > 0) {  std::swap(etai, etat); } 
    // Compute sini using Snell's law
    float sint = etai / etat * sqrt(std::max(0.f, 1 - (cosi * cosi))); 
    // Total internal reflection
    if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrt(std::max(0.f, 1 - (sint * sint))); 
        cosi = fabsf(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = ((Rs * Rs) + (Rp * Rp)) / 2; 
    } 
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
} 

vec3 refract(vec3 &I, const vec3 &N, const float &ior)
{ 
    float cosi = limit(dot(I, N),-1,1); 
    float etai = 1, etat = ior; 
    vec3 n = N; 
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= N*-1; } 
    float eta = etai / etat; 
    float k = 1 - eta * eta * (1 - cosi * cosi); 
    return k < 0 ? 0 : (I*eta) + (n*(eta * cosi - sqrtf(k))); 
}

struct ScatterInfo
{
    Ray r1;//reflection
    Ray r2;//refraction
    vec3 attenuation;
    float ior;
    float kr;
};

struct Material
{
    public:
    vec3 surfaceColor, emissionColor = 0;
    vec3 Kd, Ks; //Fator Difuso, Fator Especular, respectivamente (cor) 
    float ior, alpha;
    mat_type matType;
    std::string matName;
    float lightIntensity;

    Material();
    //Construtor para fonte de luz
    Material(const vec3 & ec): emissionColor(ec){}

    //Construtor padrao de material 
    Material(mat_type mt, const vec3 &sc = 0, const vec3 &d = 0, 
             const vec3 &s = 0, const float &a = 1, 
             const std::string &name = ""):surfaceColor(sc), Kd(d), Ks(s), alpha(a), matType(mt), matName(name) {}

    //Funcao que calcula o raio resultante da reflexao/refracao
    //virtual, pois cada material tem suas regras de reflexao/refracao
    virtual bool scatter(const Ray &r_in, vec3 &phit, vec3 &nhit, ScatterInfo &sinfo) const = 0;

};


class Diffuse : public Material {
    public:
    using Material::Material;
    Diffuse(const vec3 &sc, const vec3 &d, 
            const vec3 &s, const float &a, const std::string &name = ""):Material(mat_type::lambertian, sc, d, s, a, name), albedo(sc){}
    Diffuse(const vec3 &sc, const std::string &name):Material(mat_type::lambertian, sc, sc, vec3(0), 1, name), albedo(sc){}

    //Diffuse eh o material padrao para objetos difusos
    //scatter joga um raio em uma direcao aleatoria
    //baseada no local de impacto do raio incidente
    virtual bool scatter(const Ray& r_in, vec3 &phit, vec3 &nhit, ScatterInfo &sinfo) const  {
        vec3 target = phit + nhit + random_in_unit_sphere();
        sinfo.r1 = Ray(phit, target-phit);
        sinfo.attenuation = albedo;
        return true;
    }
    
    vec3 albedo = vec3(0);
};

class Conductor : public Material{
    public:
        Conductor(const vec3& a, float f) : albedo(a), fuzz(f) { if(f < 1) fuzz = f; else fuzz = 1; }
        Conductor(const vec3 &sc, const vec3 &d, 
        const vec3 &s, const float &a, 
        float f, const std::string &name = ""):Material(mat_type::conductor, sc, d, s, a, name), albedo(sc), fuzz(f){if(f < 1) fuzz = f; else fuzz = 1;}

        virtual bool scatter(const Ray &r_in, vec3 &phit, vec3 &nhit, ScatterInfo &sinfo) const
        {
            float bias = 1e-4;
            vec3 rdir = r_in.getDirection();
            vec3 reflectionDirection = reflect(rdir, nhit); 
            vec3 reflectionRayOrig = (dot(reflectionDirection, nhit) < 0) ? 
                phit + nhit * bias : 
                phit - nhit * bias; 
            sinfo.r1 = (Ray(reflectionRayOrig, reflectionDirection + random_in_unit_sphere()*fuzz)); 
            sinfo.r2 = Ray(0,0);
            sinfo.attenuation = albedo;
            // sinfo.kr = 1;
            // sinfo.ior = 1;
            return true;
        }

        vec3 albedo = vec3(0);
        float fuzz;
};

class Dielectric : public Material{
    public:
    //Dielectric(float ri) : Material(), ref_idx(ri) {}
    Dielectric(const float &ri = 1.5, 
               const vec3 &sc = 1,
               const std::string &name = ""):Material(mat_type::dielectric, sc, 1, 0, 1, name), ref_idx(ri){}
        virtual bool scatter(const Ray& r_in, vec3 &phit, vec3 &nhit, ScatterInfo &sinfo) const  {
            float bias = 1e-4;
            vec3 dir = r_in.getDirection();

            float kr; 
            fresnel(dir, nhit, ref_idx, kr); 
            bool outside = dot(dir, nhit) < 0;
            vec3 vbias = nhit * bias;
            if(kr < 1)
            {
                vec3 refractionDirection = unit_vector(refract(dir, nhit, ref_idx)); 
                vec3 refractionRayOrig = (outside) ? phit - vbias : phit + vbias; 
                sinfo.r2 = Ray(refractionRayOrig, refractionDirection);
            }

            vec3 reflectionDirection = unit_vector(reflect(dir, nhit)); 
            vec3 reflectionRayOrig = ( dot(reflectionDirection, nhit) < 0) ? phit - vbias : phit + vbias; 

            sinfo.ior = ref_idx;
            sinfo.kr = kr;
            sinfo.r1 = Ray(reflectionRayOrig, reflectionDirection);
            return true;
        }

        float ref_idx;
};

class Light : public Material{
    public:

        Light(const vec3 &ec, const float &str = 1,
              const std::string &name = ""):Material(ec){
                  surfaceColor = ec; matType = mat_type::light; matName = name; 
                  lightIntensity = str;}
        virtual bool scatter(const Ray& r_in, vec3 &phit, vec3 &nhit, ScatterInfo &sinfo) const 
        { return false; }
};

static Material *Chrome = new Conductor(vec3(0.25), vec3(0.4), vec3(0.774597), 76.8, 0.3);
static Material *PolishedChrome = new Conductor(vec3(0.25), vec3(0.4), vec3(0.774597), 76.8, 0.03);

static Material *Red = new Diffuse(vec3(0.65, 0.03, 0.03), vec3(0.9, 0.03, 0.03), vec3(0.2, 0.4, 0.1), 1);
static Material *Green = new Diffuse(vec3(0.12, 0.45, 0.15), vec3(0.12, 0.75, 0.15), vec3(0.4, 0.5, 0), 1);
static Material *White = new Diffuse(vec3(0.73, 0.73, 0.73), vec3(0.93, 0.93, 0.93), vec3(0), 1);
static Material *Di = new Dielectric( 2.437);
static Material *Di2 = new Dielectric( 1.5);

#endif