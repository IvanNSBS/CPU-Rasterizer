
#include <chrono>

#include <math.h>
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include "camera.h"
#include <chrono>
#include <algorithm>
#include "object.h"

// #include <SDL2/SDL.h>
#include "SDL2/SDL.h"

#define MIN 0
#define MAX 1
#define PI 3.14159265


using namespace std;

static const int WIDTH = 1000;
static const int HEIGHT = 600;

void render_scene( Mesh mesh, camera &cam, SDL_Window *wind, SDL_Renderer* renderer) {

	std::vector<Triangle> tris;
	for (int i = 0; i < mesh.vertices.size(); i += 3)
	{
		Triangle t;
		t.p[0] = mesh.vertices[i + 0];
		t.p[1] = mesh.vertices[i + 1];
		t.p[2] = mesh.vertices[i + 2];
		t.n = mesh.normals[i];
		tris.push_back(t);
	}

	std::sort(tris.begin(), tris.end(), [](Triangle& t1, Triangle &t2)
		{
            std::vector<float> d1 = { t1.p[0].z(), t1.p[1].z(), t1.p[2].z() };
            std::vector<float> d2 = { t2.p[0].z(), t2.p[1].z(), t2.p[2].z() };

            std::sort( d1.begin(), d1.end(), []( float &f1, float &f2) { return f1 > f2; } );
            std::sort( d2.begin(), d2.end(), []( float &f1, float &f2) { return f1 > f2; } );

			if( d1[0] != d2[0])
				return d1[0] < d2[0];
			else if (d1[1] != d2[1])
				return d1[1] < d2[2];
			else
				return d1[2] < d2[2];
		});


	vec3 light(-40.0f, 0.0f, -1.0f);
	light.make_unit_vector();

	for (int i = 0; i < tris.size(); i++)
	{
		tris[i].n.make_unit_vector();
		vec3 normal = tris[i].n;

		vec3 ray = tris[i].p[0] -cam._from;
		if ( dot(normal, ray ) < 0.0f || true) {

			float dp = dot(normal, light);
			vec2f praster1;
			vec2f praster2;
			vec2f praster3;

			if (cam.compute_pixel_coordinates(tris[i].p[0], praster1) &&
				cam.compute_pixel_coordinates(tris[i].p[1], praster2) &&
				cam.compute_pixel_coordinates(tris[i].p[2], praster3)
			){
				vec3 col(500, 500, 500);
				col *= dp;

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_RenderDrawLine(renderer, praster1[0], praster1[1], praster2[0], praster2[1]);
                SDL_RenderDrawLine(renderer, praster1[0], praster1[1], praster3[0], praster3[1]);
                SDL_RenderDrawLine(renderer, praster2[0], praster2[1], praster3[0], praster3[1]);

			}
		}
	}
}

void scale(float s_x, float s_y, float s_z, Mesh &mesh) {

	matrix44 tr(s_x, 0, 0, 0,
				0, s_y, 0, 0,
				0, 0, s_z, 0,
				0, 0 , 0, 1);
	for (int i = 0; i < mesh.vertices.size(); ++i) {
		tr.multVecMatrix(mesh.vertices[i], mesh.vertices[i]);
	}
	for (int i = 0; i < mesh.normals.size(); ++i) {
		tr.multVecMatrix(mesh.normals[i], mesh.normals[i]);
	}
}

void translate(vec3 tl, Mesh &mesh) {

	matrix44 tr(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				tl.x(), tl.y(), tl.z(), 1);

	for (int i = 0; i < mesh.vertices.size(); ++i) {
		tr.multVecMatrix(mesh.vertices[i], mesh.vertices[i]);
	}
	for (int i = 0; i < mesh.normals.size(); ++i) {
		tr.multVecMatrix(mesh.normals[i], mesh.normals[i]);
	}
}

void rot_y(float deg, Mesh &mesh, vec3 rot_point) {

	matrix44 tr(		1,			  0,			0,			0,
						0,			  1,			0,			0,
						0,			  0,			1,			0,
			    -rot_point.x(), -rot_point.y(), -rot_point.z(), 1);
	matrix44 itr = tr.inverse();
	float sen = sin(deg*PI / 180.0f);
	float co = cos(deg*PI / 180.0f);
	matrix44 rot(co, 0, sen, 0,
				0, 1, 0, 0,
				-sen, 0, co, 0,
				0, 0, 0, 1);
	matrix44 result = (tr*rot)*itr;

	for (int i = 0; i < mesh.vertices.size(); ++i) {
		result.multVecMatrix(mesh.vertices[i], mesh.vertices[i]);
		result.multVecMatrix(mesh.normals[i], mesh.normals[i]);
	}
}

void rot_x(float deg, Mesh &mesh, vec3 rot_point) {

	matrix44 tr(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
		-rot_point.x(), -rot_point.y(), -rot_point.z(), 1);
	matrix44 itr = tr.inverse();
	float sen = sin(deg*PI / 180.0);
	float co = cos(deg*PI / 180);
	matrix44 rot(1, 0, 0, 0,
				0, co, -sen, 0,
				0, sen, co, 0,
				0, 0, 0, 1);
	matrix44 result = (tr*rot)*itr;
	for (int i = 0; i < mesh.vertices.size(); ++i) {
		result.multVecMatrix(mesh.vertices[i], mesh.vertices[i]);
		result.multVecMatrix(mesh.normals[i], mesh.normals[i]);
	}
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;

        if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) == 0) {
            SDL_SetWindowTitle(window, "Test Title!!");
            SDL_bool done = SDL_FALSE;
            
            Mesh monkey_mesh;
            if (!monkey_mesh.load_mesh_from_file("./lowpolymountains.obj"))
                std::cout << "monkey wasnt loaded\n";

            bool mouse_down = false;
            camera cam(vec3(0, 0, 300), vec3(0, 0, -1), vec3(0, 1, 0), 60.0f, 0.1f, WIDTH, HEIGHT);

			// g++ sdl_test.cpp -IC:\mingw64\include -LC:\mingw64\lib -g -O3 -w -lmingw32 -lSDL2main -lSDL2 -o tst.exe

            while (!done) {
                SDL_Event event;

                std::clock_t then = std::clock();

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);

                // rot_x(-0.01f, monkey_mesh, monkey_mesh.bbox_center);
                // rot_y(-0.04f, monkey_mesh, monkey_mesh.bbox_center);
                render_scene(monkey_mesh, cam, window, renderer);

                SDL_RenderPresent(renderer);

                while (SDL_PollEvent(&event)) {

					float vertical_speed = 20.0f;
					float horizontal_speed = 1.0f;
					if( event.type == SDL_KEYDOWN){
						if( event.key.keysym.sym == SDLK_a )
							cam.move( vec3(horizontal_speed, 0.0f, 0.0f) );
						if( event.key.keysym.sym == SDLK_d )
							cam.move( vec3(-horizontal_speed, 0.0f, 0.0f) );
						if( event.key.keysym.sym == SDLK_w )
							cam.move( vec3(0.0f, 0.0f, -vertical_speed) );
						if( event.key.keysym.sym == SDLK_s )
							cam.move( vec3(0.0f, 0.0f, vertical_speed) );

						if( event.key.keysym.sym == SDLK_q )
							cam.move( vec3(0.00f, -vertical_speed, 0.0f) );
						if( event.key.keysym.sym == SDLK_e )
							cam.move( vec3(0.0f, vertical_speed, 0.0f) );
					}

                    if( event.type == SDL_MOUSEBUTTONDOWN )
                    {
                        //If the left mouse button was released
                        if( event.button.button == SDL_BUTTON_LEFT )
                        { 
                            //Get the mouse offsets
                            mouse_down = true;
                        }
                    }
                    if( event.type == SDL_MOUSEBUTTONUP )
                    {
                        //If the left mouse button was released
                        if( event.button.button == SDL_BUTTON_LEFT )
                        { 
                            mouse_down = false;
                        }
                    }

                    if( event.type == SDL_MOUSEMOTION && mouse_down ){
                        float x = event.motion.xrel;
                        float y = event.motion.yrel;
                        // rot_x(-y, monkey_mesh, monkey_mesh.bbox_center);
                        // rot_y(-x, monkey_mesh, monkey_mesh.bbox_center);
						
						//WORKING!
						// cam.rot_x(y*0.003f);
						
						
						cam.rot_y(x*0.003f);
                    }

                    if (event.type == SDL_QUIT) {
                        done = SDL_TRUE;
                    }
					
                }

                std::clock_t now = std::clock();
                double ms = (double)(now - then)/CLOCKS_PER_SEC;
                // ms *= 1000.0;
                ms = 1.0/ms;
                // std::cout << "frametime: " << (int)ms << "\n";
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    SDL_Quit();
    return 0;
}
