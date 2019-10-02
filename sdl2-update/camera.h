#ifndef CAMERAH
#define CAMERAH

#include "vec3.h"
#include "vec2.h"
#include "matrix44.h"
#include "object.h"
#include <algorithm>

#ifdef _WIN32 || WIN32
	#include <SDL.h>
#elif defined(__unix__)
	#include <SDL2/SDL.h>
#endif

const int WIDTH = 600;
const int HEIGHT = 400;
static const int INSIDE = 0; // 0000
static const int LEFT = 1;   // 0001
static const int RIGHT = 2;  // 0010
static const int BOTTOM = 4; // 0100
static const int TOP = 8;    // 1000

class camera
{
public:
    int imgWidth, imgHeight;
    float fov, _near;
    float bottom, left, top, right;
    matrix44 camToWorld;
    matrix44 worldToCamera;
    float _far = 1000.0f;
	vec3 _from, _at, _up;
    vec3 axisX, axisY, axisZ;
    vec3 rotation = vec3(0,0,0);

    std::vector<float> z_buffer;
public:
    camera();
    camera(
        const vec3 &from, const vec3 &at, const vec3 &up,
        const float &f, const float &n,
        const int &iwidth, const int &iheight): 
        fov(f), _near(n), imgWidth(iwidth), imgHeight(iheight),
        _from(from), _at(at), _up(up)
    {
        float aspectratio = iwidth/(float)iheight;
        float angle = std::tan((f*0.5f)*3.14f/180.0f);
        top = angle; 
        right = angle * aspectratio;    
        bottom = -top; 
        left = -right;
        printf("l = %f, r = %f, t = %f, b = %f\n", left, right, top, bottom);
        set_axis_and_matrix(from, at, up, true);
        reset_zbuffer();
    }

    void reset_zbuffer(){ 
        z_buffer.clear();
        z_buffer.reserve(WIDTH*HEIGHT);
        for(int i = 0; i < WIDTH*HEIGHT; i++)
            z_buffer.push_back(100000.0f);
    }

    void set_axis_and_matrix(const vec3 &from, const vec3 &at, const vec3 &up, bool update_axis = false)
    {
        if( update_axis ){
            axisZ = unit_vector( from-at );
            axisY = unit_vector( up - ( axisZ * ( dot(up,axisZ)/dot(axisZ, axisZ) ) ) );
            axisX = unit_vector( cross(axisZ, axisY) );
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

    bool compute_pixel_coordinates(const vec3 &pWorld, vec2 &pRaster, float &z) 
    { 
		// vec3 ray = pWorld - _from;
		// ray.make_unit_vector();
		// vec3 lookdir = axisZ;
		// if ( dot(ray, lookdir) >= 0.0f )
		// 	return false;

        // vec3 vertexCamera;
        // worldToCamera.mult_point_matrix(pWorld, vertexCamera); 

        // // convert to screen space
        // vec2 vertexScreen; 
        // vertexScreen[0] = _near * vertexCamera.x() / -vertexCamera.z(); 
        // vertexScreen[1] = _near * vertexCamera.y() / -vertexCamera.z(); 
    
        // // now convert point from screen space to NDC space (in range [-1,1])
        // vec2 vertexNDC; 
        // vertexNDC[0] = 2.0 * vertexScreen.x() / (right - left) - (right + left) / (right - left); 
        // vertexNDC[1] = 2.0 * vertexScreen.y() / (top - bottom) - (top + bottom) / (top - bottom); 
    
        // // convert to raster space
        // pRaster[0] = (vertexNDC.x() + 1) / 2 * imgWidth; 
        // // in raster space y is down so invert direction
        // pRaster[1] = (1 - vertexNDC.y()) / 2 * imgHeight; 
        // z = -vertexCamera.z();
        // return true;

        vec3 toObj = vec3(pWorld.x() - _from.x(), pWorld.y() - _from.y(), pWorld.z() - _from.z());
        toObj.make_unit_vector();
        if (dot(toObj, axisZ) > 0) {
            return false; //Retorna falso se o ponto estiver atr�s da camera
        }
 
        vec3 pScreen;
        worldToCamera.mult_point_matrix(pWorld, pScreen);
        z = -pScreen.z();
 
        vec3 ccdc = vec3(pScreen.x() * (_near / pScreen.z()), pScreen.y() * (_near / pScreen.z()), _near);
 
        matrix44 auxMatriz = matrix44((2 * _near) / (right - left), 0, 0, 0,
            0, (2 * _near) / (bottom - top), 0, 0,
            -(right + left) / (right - left), -(bottom + top) / (bottom - top), (_far + _near) / (_far - _near), 1,
            0, 0, -(2 * _near) / (_far - _near), 0);
 
        vec3 pndc;
        auxMatriz.mult_point_matrix(ccdc, pndc);

 
        pRaster = vec2((1 + pndc.x()) / 2 * imgWidth, (1 - pndc.y()) / 2 * imgHeight);
        if (pndc.y() <= top && pndc.y() >= bottom && pndc.x() <= right && pndc.x() >= left)
            return true;

        return false; // Retornar verdadeiro se o ponto pode ser visto

    }

    void rotate(float dx, float dy)
    {
        vec3 rot(dx, dy, 0);
        rotation += vec3(dx, dy, 0);
        camToWorld.mult_vec_matrix(rot, rot);
        _at += rot;
        set_axis_and_matrix(_from, _at, _up, true);
    }

    void move(vec3 dir) {
        camToWorld.mult_vec_matrix(dir, dir);
		_from += dir;
		_at += dir;
		set_axis_and_matrix( _from, _at, _up, false);
	}

    bool clip_line( vec2 &p1, vec2 &p2, vec3 *outcol)
    {
        int ymin = 0, ymax = HEIGHT;
        int xmin = 0, xmax = WIDTH;
        int codep1 = 0000, codep2 = 0000;
        
        auto get_outcode = [&]( int &code, const vec2 &p){
            code = INSIDE;          // initialised as being inside of [[clip window]]

            if (p.x() < xmin)           // to the left of clip window
                code |= LEFT;
            if (p.x() > xmax)      // to the right of clip window
                code |= RIGHT;
            if (p.y() < ymin)           // below the clip window
                code |= BOTTOM;
            if (p.y() > ymax)      // above the clip window
                code |= TOP;
        };


        get_outcode(codep1, p1);
        get_outcode(codep2, p2);

        bool accept = false;
        while(true){
            if( codep1 == 0 && codep2 == 0 ){
                accept = true; // trivialmente aceito
                break;
            }
            else if(codep1 & codep2) // trivialmente recusado
                break;
            else{
                // failed both tests, so calculate the line segment to clip
                // from an outside point to an intersection with clip edge
                float x, y;
                // At least one endpoint is outside the clip rectangle; pick it.
                int codeOut;
                if( codep1 != 0)
                    codeOut = codep1;
                else
                    codeOut = codep2;

                // Now find the intersection point;
                // use formulas:
                //   slope = (p1[1] - p0[1]) / (x1 - x0)
                //   x = x0 + (1 / slope) * (ym - p0[1]), where ym is 0 or HEIGHT
                //   y = p0[1] + slope * (xm - x0), where xm is xmin or WIDTH
                // No need to worry about divide-by-zero because, in each case, the
                // outcode bit being tested guarantees the denominator is non-zero
                float slope = (p2.y() - p1.y())/(p2.x() - p1.x());
                if (codeOut & TOP) {           // point is above the clip window
                    x = p1.x() + (1.0/slope)*(ymax - p1.y());
                    y = ymax;
                } 
                else if (codeOut & BOTTOM) { // point is below the clip window
                    x = p1.x() + (1.0/slope)*(ymin - p1.y());
                    y = ymin;
                } 
                else if (codeOut & RIGHT) {  // point is to the right of clip window
                    y = p1.y() + slope*(xmax-p1.x());
                    x = xmax;
                } 
                else if (codeOut & LEFT) {   // point is to the left of clip window
                    y = p1.y() + slope*(xmin-p1.x());
                    x = xmin;
                }

                // Now we move outside point to intersection point to clip
                // and get ready for next pass.
                if (codeOut == codep1) {
                    p1[0] = x;
                    p1[1] = y;
                    get_outcode(codep1, p1);
                } 
                else {
                    p2[0] = x;
                    p2[1] = y;
                    get_outcode(codep2, p2);
                }
            }
        }
        return accept;
    }

    void fill_triangle(
        SDL_Renderer* renderer, 
        const vec2 &v0, 
        const vec2 &v1, 
        const vec2 &v2,
        const Vertex *vert,
        float *screen_z,
        const vec3 &light_dir,
        const Obj *obj = nullptr)
    {
        auto edge_function = [](const vec2 &p, const vec2 &v0, const vec2& v1, float &out) -> bool
        {
            out = (p.x() - v0.x())*(v1.y() - v0.y()) - (p.y() - v0.y())*(v1.x() - v0.x());
            return out >= 0; 
        };
        
        auto get_bbox = [](
            const vec2 &v0,
            const vec2 &v1,
            const vec2 &v2,
            vec2 &mn, 
            vec2 &mx)
        {
            float minx = 100000, miny = 100000, maxx = -100000, maxy = -100000;
            std::vector<vec2> tri;
            tri.reserve(3);
            tri.push_back(v0);
            tri.push_back(v1);
            tri.push_back(v2);
            for(int i = 0; i < 3; i++)
            {
                if( tri[i].x() < minx)
                    minx = tri[i].x();
                if( tri[i].y() < miny)
                    miny = tri[i].y();

                if( tri[i].x() > maxx)
                    maxx = tri[i].x();
                if( tri[i].y() > maxy)
                    maxy = tri[i].y();
            }
            mn = vec2(minx,miny);
            mx = vec2(maxx,maxy);
        };

        vec2 min, max;
        get_bbox(v0, v1, v2, min, max);
        vec2 cur_xy;
        float out_area;
        float w0, w1, w2;
        edge_function(v0, v1, v2, out_area);

        // printf("screen z[0] = %f\nscreen_z[1] = %f\nscreen_z[2] = %f\n", screen_z[0], screen_z[1], screen_z[2]);

        screen_z[0] = 1.0/screen_z[0]; 
        screen_z[1] = 1.0/screen_z[1]; 
        screen_z[2] = 1.0/screen_z[2]; 

        vec2 st0 = vert[0].uv*screen_z[0];
        vec2 st1 = vert[1].uv*screen_z[1];
        vec2 st2 = vert[2].uv*screen_z[2];

        vec2 st;

        for(int y = min.y(); y < max.y(); y++)
        {
            for(int x = min.x(); x < max.x(); x++)
            {
                cur_xy = vec2(x+0.5,y+0.5);
                bool c1 = edge_function( cur_xy, v0, v1, w2 );
                bool c2 = edge_function( cur_xy, v1, v2, w0 );
                bool c3 = edge_function( cur_xy, v2, v0, w1 );
                if(c1 && c2 && c3)
                {
                    w0 /= out_area;
                    w1 /= out_area;
                    w2 /= out_area;
                    float z = w0*(screen_z[0]) + w1*(screen_z[1]) + w2*(screen_z[2]);
                    z = 1.0f/z;
                    if(z < z_buffer[y*WIDTH + x])
                    {
                        z_buffer[y*WIDTH + x] = z;
                        vec3 normal = w0*vert[0].normal + w1*vert[1].normal + w2*vert[2].normal;
                        st = w0*st0 + w1*st1 + w2*st2;
                        st *= z;
                        float p = (fmod(st.x() * 10, 1.0) > 0.5) ^ (fmod(st.y() * 10, 1.0) < 0.5);
                        float val = std::max(0.0f, -dot(normal, light_dir));

                        // vec3 color = p*val*vec3(210,210,210) + vec3(40, 40, 40);
                        int tx = std::floor( st.x()*(float)obj->texture_width);
                        int ty = std::floor( st.y()*(float)obj->texture_height);
                        int idx = (ty*obj->texture_width + tx);
                        // printf("s = %f, t = %f\n", st.x(), st.y());
                        // printf("idx = %d\n", idx);
                        vec3 color = obj->texture_buffer[idx] * val + vec3(40,40,40);
                        color[0] = std::min(color[0], 255.f);
                        color[1] = std::min(color[1], 255.f);
                        color[2] = std::min(color[2], 255.f);

                        SDL_SetRenderDrawColor(renderer, color.x(), color.y(), color.z(), 255);
                        SDL_RenderDrawPoint(renderer, cur_xy.x(), cur_xy.y());
                    }
                }
            }
        }
    }

    void draw_lines( const vec2 &p0, const vec2 &p1, SDL_Renderer *renderer, const vec3 &col)
    {
		int steps = (p0-p1).length();
		vec2 dir = unit_vector(p0 - p1);
		vec2 start = p1;
		for(int i = 0; i <= steps; i++){ 
			SDL_RenderDrawPoint(renderer, start.x(), start.y());
			start += dir;
		} 
    }

    void render_scene( std::vector<Obj> objs, SDL_Renderer* renderer) {

        vec3 light(0.0f, 0.0f, -1.0f);
        light.make_unit_vector();

        reset_zbuffer();
        for (auto obj : objs){
            for (int i = 0; i < obj.mesh.tris.size(); i++)
            {
                vec3 n1 = obj.mesh.tris[i].vertex[0].normal;
                vec3 n2 = obj.mesh.tris[i].vertex[1].normal;
                vec3 n3 = obj.mesh.tris[i].vertex[2].normal;
                vec3 normal = (1.f/3.f) * n1 + (1.f/3.f)*n2 + (1.f/3.f)*n3;

                vec3 ray = obj.mesh.tris[i].vertex[0].pos - _from;
                    float dp = dot(normal, light);
                    float z1, z2, z3;
                    vec2 praster1;
                    vec2 praster2;
                    vec2 praster3;

                    vec3 col(255, 255, 255);
                    SDL_SetRenderDrawColor(renderer, obj.col.x(), obj.col.y(), obj.col.z(), SDL_ALPHA_OPAQUE);

                    bool v1, v2, v3;
                    v1 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[0].pos, praster1, z1);
                    v2 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[1].pos, praster2, z2);
                    v3 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[2].pos, praster3, z3);
                    
                    float zs[3] = { z1, z2, z3 };

                    if( v1 && v2 && v3)
                        fill_triangle(renderer, praster1, praster2, praster3, obj.mesh.tris[i].vertex, zs, light, &obj);

                    // if(v1 && v2){
                    //     bool clip = false;
                    //     vec2 raster1 = praster1;
                    //     vec2 raster2 = praster2;

                    //     clip = clip_line(raster1, raster2, 0);
                    //     if(clip)
                    //         draw_lines(raster1, raster2, renderer, col);
                        
                    // }
                    // if(v1 && v3){
                    //     bool clip = false;
                    //     vec2 raster1 = praster1;
                    //     vec2 raster3 = praster3;

                    //     clip = clip_line(raster1, raster3, 0);
                    //     if(clip)
                    //         draw_lines(raster1, raster3, renderer, col);
                    // }
                    // if(v2 && v3){
                    //     bool clip = false;
                    //     vec2 raster2 = praster2;
                    //     vec2 raster3 = praster3;

                    //     clip = clip_line(raster2, raster3, 0);
                    //     if(clip)
                    //         draw_lines(raster2, raster3, renderer, col);

                    // }

            }
        }
    }

};


#endif