
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

#include "ImGUI/imgui_sdl.h"
#include "ImGUI/imgui.h"

#ifdef _WIN32 || WIN32
	#include "SDL2/SDL.h"
#elif defined(__unix__)
	#include <SDL2/SDL.h>
#endif

#define MIN 0
#define MAX 1
#define PI 3.14159265


using namespace std;

static const int WIDTH = 1000;
static const int HEIGHT = 600;

void render_scene( std::vector<Obj> objs, camera &cam, SDL_Window *wind, SDL_Renderer* renderer) {

	for (auto obj : objs){
		std::sort(obj.mesh.tris.begin(), obj.mesh.tris.end(), [](Triangle& t1, Triangle &t2)
			{
				std::vector<float> d1 = { t1.vert[0].z(), t1.vert[1].z(), t1.vert[2].z() };
				std::vector<float> d2 = { t2.vert[0].z(), t2.vert[1].z(), t2.vert[2].z() };

				std::sort( d1.begin(), d1.end(), []( float &f1, float &f2) { return f1 > f2; } );
				std::sort( d2.begin(), d2.end(), []( float &f1, float &f2) { return f1 > f2; } );

				if( d1[0] != d2[0])
					return d1[0] < d2[0];
				else if (d1[1] != d2[1])
					return d1[1] < d2[2];
				else
					return d1[2] < d2[2];
			});
	}


	vec3 light(-40.0f, 0.0f, -1.0f);
	light.make_unit_vector();

	for (auto obj : objs){
		for (int i = 0; i < obj.mesh.tris.size(); i++)
		{
			obj.mesh.tris[i].normal.make_unit_vector();
			vec3 normal = obj.mesh.tris[i].normal;

			vec3 ray = obj.mesh.tris[i].vert[0] -cam._from;
			if ( dot(normal, ray ) < 0.0f || true) {

				float dp = dot(normal, light);
				vec2f praster1;
				vec2f praster2;
				vec2f praster3;

				if (cam.compute_pixel_coordinates(obj.mesh.tris[i].vert[0], praster1) &&
					cam.compute_pixel_coordinates(obj.mesh.tris[i].vert[1], praster2) &&
					cam.compute_pixel_coordinates(obj.mesh.tris[i].vert[2], praster3)
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
}



int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;

        if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) == 0) {
            SDL_SetWindowTitle(window, "Test Title!!");
            SDL_bool done = SDL_FALSE;
            
			std::vector<Obj> objects;
            objects.push_back( Obj("./monkey.obj") );
            objects.push_back( Obj("./monkey.obj") );

			// monkey_mesh.scale( vec3(0.1, 0.1, 0.1) );
			objects[0].translate(vec3(2.0f, 0.0f, 0.0f));

			ImGui::CreateContext();
			ImGuiSDL::Initialize(renderer, 800, 600);

            bool mouse_down = false;
            camera cam(vec3(0, 0, 300), vec3(0, 0, -1), vec3(0, 1, 0), 60.0f, 0.1f, WIDTH, HEIGHT);

			// g++ sdl_test.cpp -IC:\mingw64\include -LC:\mingw64\lib -g -O3 -w -lmingw32 -lSDL2main -lSDL2 -o tst.exe
			double ms = -1;

			float my_color[4];
			bool my_tool_active;
            while (!done) {
                SDL_Event event;
				ImGuiIO& io = ImGui::GetIO();
                std::clock_t then = std::clock();

				int mouseX, mouseY;
				const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

				// Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)
				
				io.DeltaTime = 1.0f / 60.0f;
				io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
				io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
				io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);


				ImGui::NewFrame();
			
				//ImGui::ShowDemoWindow();

				// Create a window called "My First Tool", with a menu bar.
				ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
						if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */ }
						if (ImGui::MenuItem("Close", "Ctrl+W"))  { my_tool_active = false; }
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				// Edit a color (stored as ~4 floats)
				ImGui::ColorEdit4("Color", my_color);

				// Plot some values
				const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
				ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

				// Display contents in a scrolling region
				ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
				ImGui::BeginChild("Scrolling");
				for (int n = 0; n < 1; n++)
					ImGui::Text("FPS: %d", (int)ms);
				ImGui::EndChild();
				ImGui::End();

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
				SDL_RenderClear(renderer);

                render_scene(objects, cam, window, renderer);

				ImGui::Render();
				ImGuiSDL::Render(ImGui::GetDrawData());
                SDL_RenderPresent(renderer);

				std::clock_t now = std::clock();
                ms = (double)(now - then);
				ms /= CLOCKS_PER_SEC;
				//monkey_mesh.rot_x(1.f * ms);
				//monkey_mesh.rot_y(1.f * ms);
                std::cout << "frametime: " << ms << "\n";

                while (SDL_PollEvent(&event)) {

					float vertical_speed = 1.0f;	
					float up_speed = 0.2f;
					float horizontal_speed = 0.10f;
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
							cam.move( vec3(0.00f, -up_speed, 0.0f) );
						if( event.key.keysym.sym == SDLK_e )
							cam.move( vec3(0.0f, up_speed, 0.0f) );
					}

                    if( event.type == SDL_MOUSEBUTTONDOWN )
                    {
                        //If the left mouse button was released
                        if( event.button.button == SDL_BUTTON_RIGHT )
                        { 
                            //Get the mouse offsets
                            mouse_down = true;
                        }
                    }
                    if( event.type == SDL_MOUSEBUTTONUP )
                    {
                        //If the left mouse button was released
                        if( event.button.button == SDL_BUTTON_RIGHT )
                        { 
                            mouse_down = false;
                        }
                    }

                    if( event.type == SDL_MOUSEMOTION && mouse_down ){
                        float x = event.motion.xrel;
                        float y = event.motion.yrel;
                       	//rot_x(-y, monkey_mesh, monkey_mesh.bbox_center);
                        //rot_y(-x, monkey_mesh, monkey_mesh.bbox_center);
						
						//WORKING!
						cam.rot_x(y * 0.02f * ms);
						cam.rot_y(x * 0.02f * ms);
                    }

                    if (event.type == SDL_QUIT) {
                        done = SDL_TRUE;
                    }
					
                }

                ms = 1.0/ms;

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
