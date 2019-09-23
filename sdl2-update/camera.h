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
            z_buffer.push_back(-100000.0f);
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

    bool compute_pixel_coordinates(const vec3 &pWorld, vec2 &praster, vec3 *screen = nullptr) 
    { 
        vec3 toObj = vec3(pWorld.x() - _from.x(), pWorld.y() - _from.y(), pWorld.z() - _from.z());
        toObj.make_unit_vector();
        if (dot(toObj, axisZ) > 0) {
            return false; //Retorna falso se o ponto estiver atr�s da camera
        }
 
        vec3 pScreen;
        worldToCamera.mult_point_matrix(pWorld, pScreen);

 
        vec3 ccdc = vec3(pScreen.x() * (_near / pScreen.z()), pScreen.y() * (_near / pScreen.z()), _near);
 
        matrix44 auxMatriz = matrix44((2 * _near) / (right - left), 0, 0, 0,
            0, (2 * _near) / (bottom - top), 0, 0,
            -(right + left) / (right - left), -(bottom + top) / (bottom - top), (_far + _near) / (_far - _near), 1,
            0, 0, -(2 * _near) / (_far - _near), 0);
 
        vec3 pndc;
        auxMatriz.mult_point_matrix(ccdc, pndc);
        if(screen)
            screen = new vec3(0, 0, -pScreen.z());
 
        praster = vec2((1 + pndc.x()) / 2 * imgWidth, (1 - pndc.y()) / 2 * imgHeight);
        if (pndc.y() <= top && pndc.y() >= bottom && pndc.x() <= right && pndc.x() >= left) {
            return true;
        }
        else {
            return false; // Retornar verdadeiro se o ponto pode ser visto
        }
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
        const float *screen_z,
        const vec3 &light_dir)
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
        
        for(int y = min.y(); y < max.y(); y++)
        {
            for(int x = min.x(); x < max.x(); x++)
            {
                cur_xy = vec2(x,y);
                bool c1 = edge_function( cur_xy, v0, v1, w0 );
                bool c2 = edge_function( cur_xy, v1, v2, w1 );
                bool c3 = edge_function( cur_xy, v2, v0, w2 );
                if(c1 && c2 && c3)
                {
                    // float z = w1*(screen_z[0]) + w2*(screen_z[1]) + w0*(screen_z[2]);
                    //z = 1.0f/z;
                    // float z = (screen_z[0] + screen_z[1] + screen_z[2])/3.0f;
                    float z = (vert[0].pos.z() + vert[1].pos.z() + vert[2].pos.z())/3.0f;
                    // float z = w1*vert[0].pos.z() + w2*vert[1].pos.z() + w0*vert[2].pos.z();
                    if(z > z_buffer[y*WIDTH + x])
                    {
                        z_buffer[y*WIDTH + x] = z;
                        w0 /= out_area;
                        w1 /= out_area;
                        w2 /= out_area;
                        vec3 normal = w1*vert[0].normal + w2*vert[1].normal + w0*vert[2].normal;
                        float val = std::max(0.0f, -dot(normal, light_dir));
                        vec3 color = val*vec3(210,210,210) + vec3(40, 40, 40);
                        //  printf("z = %f\n", z);
                        SDL_SetRenderDrawColor(renderer, color.x(), color.y(), color.z(), 255);
                        SDL_RenderDrawPoint(renderer, cur_xy.x(), cur_xy.y());
                    }
                }
            }
        }
    }

    void draw_lines( const vec2 &p0, const vec2 &p1, SDL_Renderer *renderer, const vec3 &col)
    {
        // calculate dx , dy
        int dx = p1.x() - p0.x();
        int dy = p1.y() - p0.y();
        if( dx == 0 && dy == 0)
            return;

        // Depending upon absolute value of dx & dy
        // choose number of steps to put pixel as
        // steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy)
        // int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

        // // calculate increment in x & y for each steps
        // float Xinc = dx / (float) steps;
        // float Yinc = dy / (float) steps;

        // // Put pixel for each step
        // float X = p0.x();
        // float Y = p0.y();
        
		// if(steps > 50)
		// 	printf("steps = %d\n", steps);
        // for (int i = 0; i <= steps; i++)
        // {
        //     SDL_RenderDrawPoint(renderer, X, Y);
        //     X += Xinc;
        //     Y += Yinc;
        // }

		// int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
		int steps = (p0-p1).length();
		vec2 dir = unit_vector(p0 - p1);
		vec2 start = p1;
		for(int i = 0; i <= steps; i++){ 
			SDL_RenderDrawPoint(renderer, start.x(), start.y());
			start += dir;
		} 

        // int dx = p1.x() - p0.x();
        // int dy = p1.y() - p0.y();
        // // points are too close, draw point and return
        // if( dx == 0 && dy == 0){
        // 	SDL_RenderDrawPoint(renderer, p1.x(), p1.y());
        // 	return;
        // }

        // vec2 origin = p0;
        // vec2 line = unit_vector(p1 - p0);
        // float end, start, step;
        // if( dx != 0 ){
        // 	end = p1.x() > p0.x() ? p1.x() : p0.x();
        // 	start = p1.x() > p0.x() ? p0.x() : p1.x();
        // 	step = abs(line.x());
        // }
        // else{ 
        // 	end = p1.y() > p0.y() ? p1.y() : p0.y();
        // 	start = p1.y() > p0.y() ? p0.y() : p1.y();
        // 	step = abs(line.y());	
        // }

        // // printf("start = %f | end = %f | step = %f\n", start, end, step);

        // unsigned char a = 0000;
        // for(float i = start; i <= end; i+=step){
        // 	SDL_RenderDrawPoint(renderer, origin.x(), origin.y());
        // 	origin += line;
        // }
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
                if ( dot(normal, ray ) <= 0.0f ) {

                    float dp = dot(normal, light);
                    vec3 *screen1 = new vec3(), *screen2 = new vec3(), *screen3 = new vec3();
                    vec2 praster1;
                    vec2 praster2;
                    vec2 praster3;

                    vec3 col(255, 255, 255);
                    SDL_SetRenderDrawColor(renderer, obj.col.x(), obj.col.y(), obj.col.z(), SDL_ALPHA_OPAQUE);

                    bool v1, v2, v3;
                    v1 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[0].pos, praster1, screen1);
                    v2 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[1].pos, praster2, screen2);
                    v3 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[2].pos, praster3, screen3);
                    
                    float zs[3] = { (*screen1).z(), (*screen2).z(), (*screen3).z() };

                    if( v1 && v2 && v3)
                        fill_triangle(renderer, praster1, praster2, praster3, obj.mesh.tris[i].vertex, zs, light);

                }
            }
        }
    }

};


#endif