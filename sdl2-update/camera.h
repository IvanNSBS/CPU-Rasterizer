#ifndef CAMERAH
#define CAMERAH

#include "vec3.h"
#include "vec2.h"
#include "matrix44.h"

#ifdef _WIN32 || WIN32
	#include <SDL.h>
#elif defined(__unix__)
	#include <SDL2/SDL.h>
#endif

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

    void set_axis_and_matrix(const vec3 &from, const vec3 &at, const vec3 &up, bool update_axis = false)
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

    bool compute_pixel_coordinates(const vec3 &pWorld, vec2 &pRaster) 
    { 
		vec3 ray = pWorld - _from;
		ray.make_unit_vector();
		vec3 lookdir = axisZ;
		if ( dot(ray, lookdir) > 0.0f )
			return false;

        vec3 pCamera; 
        vec2 pScreen; 
        worldToCamera.mult_point_matrix(pWorld, pCamera); 
        
        pScreen[0] = pCamera.x() * _near / (-pCamera.z()); 
        pScreen[1] = pCamera.y() * _near / (-pCamera.z()); 
    
        vec2 pNDC; 
        pNDC[0] = (pScreen.x() + right) / (2 * right); 
        pNDC[1] = (pScreen.y() + top) / (2 * top); 
        pRaster[0] = (pNDC.x() * imgWidth); 
        pRaster[1] = ((1 - pNDC.y()) * imgHeight); 
    
        bool visible = true; 
        /*if (pScreen.x() < left || pScreen.x() > right || pScreen.y() < bottom || pScreen.y() > top) 
            visible = false;*/ 
    
        return visible; 
    }

    void rotate( float dx, float dy)
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

    bool clip_line( vec2 &p0, vec2 &p1)
    {
        int t = 0, b = HEIGHT;
        int l = 0, r = WIDTH;
        int codep0 = 0000, codep1 = 0000;
        
        auto get_outcode = [&t, &l, &b, &r]( int &code, const vec2 &p){
            code = INSIDE;          // initialised as being inside of [[clip window]]

            if (p.x() < 0)           // to the left of clip window
                code |= LEFT;
            else if (p.x() > WIDTH)      // to the right of clip window
                code |= RIGHT;
            if (p.y() < 0)           // below the clip window
                code |= BOTTOM;
            else if (p.y() > HEIGHT)      // above the clip window
                code |= TOP;
        };

        get_outcode(codep0, p0);
        get_outcode(codep1, p1);

        bool accept = false;
        unsigned int changed = 0;
        while(true){
            if( !(codep0 | codep1) ){
                accept = true;
                return changed > 0; // trivialmente aceito
            }
            else if(codep0 & codep1) // trivialmente recusado
                return false;
            else{
                // failed both tests, so calculate the line segment to clip
                // from an outside point to an intersection with clip edge
                float x, y;
                changed++;
                // At least one endpoint is outside the clip rectangle; pick it.
                unsigned char codeOut = codep0 ? codep0 : codep1;

                // Now find the intersection point;
                // use formulas:
                //   slope = (p1[1] - p0[1]) / (x1 - x0)
                //   x = x0 + (1 / slope) * (ym - p0[1]), where ym is 0 or HEIGHT
                //   y = p0[1] + slope * (xm - x0), where xm is xmin or WIDTH
                // No need to worry about divide-by-zero because, in each case, the
                // outcode bit being tested guarantees the denominator is non-zero
                if (codeOut & TOP) {           // point is above the clip window
                    x = p0[0] + (p1[0] - p0[0]) * (HEIGHT - p0[1]) / (p1[1] - p0[1]);
                    y = HEIGHT;
                } else if (codeOut & BOTTOM) { // point is below the clip window
                    x = p0[0] + (p1[0] - p0[0]) * (0 - p0[1]) / (p1[1] - p0[1]);
                    y = 0;
                } else if (codeOut & RIGHT) {  // point is to the right of clip window
                    y = p0[1] + (p1[1] - p0[1]) * (WIDTH - p0[0]) / (p1[0] - p0[0]);
                    x = WIDTH;
                } else if (codeOut & LEFT) {   // point is to the left of clip window
                    y = p0[1] + (p1[1] - p0[1]) * (0 - p0[0]) / (p1[0] - p0[0]);
                    x = 0;
                }

                // Now we move outside point to intersection point to clip
                // and get ready for next pass.
                if (codeOut == codep0) {
                    p0[0] = x;
                    p0[1] = y;
                    get_outcode(codep0, p0);
                } else {
                    p1[0] = x;
                    p1[1] = y;
                    get_outcode(codep1, p1);
                }
            }
        }
        return accept;
    }

    void draw_lines( const vec2 &p0, const vec2 &p1, SDL_Renderer *renderer, const vec3 &col)
    {

        // calculate dx , dy
        int dx = p1.x() - p0.x();
        int dy = p1.y() - p0.y();

        // Depending upon absolute value of dx & dy
        // choose number of steps to put pixel as
        // steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy)
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

        // calculate increment in x & y for each steps
        float Xinc = dx / (float) steps;
        float Yinc = dy / (float) steps;

        // Put pixel for each step
        float X = p0.x();
        float Y = p0.y();
        for (int i = 0; i <= steps; i++)
        {
            SDL_RenderDrawPoint(renderer, X, Y);
            X += Xinc;
            Y += Yinc;
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

    void render_scene( std::vector<Obj> objs, SDL_Window *wind, SDL_Renderer* renderer) {

        // for (auto obj : objs){
        // 	std::sort(obj.mesh.tris.begin(), obj.mesh.tris.end(), [](Triangle& t1, Triangle &t2)
        // 		{
        // 			std::vector<float> d1 = { t1.vert[0].z(), t1.vert[1].z(), t1.vert[2].z() };
        // 			std::vector<float> d2 = { t2.vert[0].z(), t2.vert[1].z(), t2.vert[2].z() };

        // 			std::sort( d1.begin(), d1.end(), []( float &f1, float &f2) { return f1 > f2; } );
        // 			std::sort( d2.begin(), d2.end(), []( float &f1, float &f2) { return f1 > f2; } );

        // 			if( d1[0] != d2[0])
        // 				return d1[0] < d2[0];
        // 			else if (d1[1] != d2[1])
        // 				return d1[1] < d2[2];
        // 			else
        // 				return d1[2] < d2[2];
        // 		});
        // }

        vec3 light(0.0f, 0.0f, -1.0f);
        light.make_unit_vector();

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
                    vec2 praster1;
                    vec2 praster2;
                    vec2 praster3;

                    vec3 col(255, 255, 255);
                    SDL_SetRenderDrawColor(renderer, obj.col.x(), obj.col.y(), obj.col.z(), SDL_ALPHA_OPAQUE);

                    bool v1, v2, v3;
                    v1 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[0].pos, praster1);
                    v2 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[1].pos, praster2);
                    v3 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[2].pos, praster3);


                    if(v1 && v2){
                        bool clip = clip_line(praster1, praster2);
                        if(clip)
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                        draw_lines(praster1, praster2, renderer, col);
                        // SDL_RenderDrawLine(renderer, praster1[0], praster1[1], praster2[0], praster2[1]);
                    }
                    if(v1 && v3){
                        bool clip = clip_line(praster1, praster3);
                        if(clip)
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                        // SDL_RenderDrawLine(renderer, praster1[0], praster1[1], praster3[0], praster3[1]);
                        draw_lines(praster1, praster3, renderer, col);
                    }
                    if(v2 && v3){
                        bool clip = clip_line(praster2, praster3);
                        if(clip)
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                        // SDL_RenderDrawLine(renderer, praster2[0], praster2[1], praster3[0], praster3[1]);
                        draw_lines(praster2, praster3, renderer, col);
                    }
                }
            }
        }
    }

};


#endif