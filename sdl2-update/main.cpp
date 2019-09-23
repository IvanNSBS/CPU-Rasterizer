
#include <string>
#include <math.h>
#include "camera.h"
#include <chrono>

#include "ImGUI/imgui_sdl.h"
#include "ImGUI/imgui.h"

#define MIN 0
#define MAX 1
#define PI 3.14159265


bool intersects_triangle( Triangle &tr, vec3 ray_org, vec3 ray_dir, vec3 &out_col, vec3 &out_point, float &t)
{
    const float EPSILON = 0.0000001;
    // vec3 vertex0 = tr.vertex[0].pos;
    // vec3 vertex1 = tr.vertex[1].pos;  
    // vec3 vertex2 = tr.vertex[2].pos;
    // vec3 edge1, edge2, h, s, q;
    // float a,f,u,v;
    // edge1 = vertex1 - vertex0;
    // edge2 = vertex2 - vertex0;
    // h = cross(ray_dir, edge2);
    // a = dot( edge1, h );
    // if (a > -EPSILON && a < EPSILON){
    //     return false;    // This ray is parallel to this triangle.
	// }
    // f = 1.0/a;
    // s = ray_org - vertex0;
    // u = f *dot(s,h);
    // if (u < 0.0 || u > 1.0)
    //     return false;
    // q = cross(s, edge1);
    // v = f * dot(ray_dir, q);
    // if (v < 0.0 || u + v > 1.0)
    //     return false;
    // // At this stage we can compute t to find out where the intersection point is on the line.
    // t = f * dot(edge2, q);
    // if (t > EPSILON) // ray intersection
    // {
	// 	float w = 1.0 - u - v;
	// 	vec3 normal = w*tr.vertex[0].normal + u*tr.vertex[1].normal + v*tr.vertex[1].normal; 
	// 	out_col = vec3(0, 255, 0) * (dot(normal, vec3(0,0, -1.0f)));
    //     out_point = ray_org + ray_dir * t;
    //     return true;
    // }
    // else {
    //     return false;
	// }// This means that there is a line intersection but not a ray intersection.
	vec3 v0 = tr.vertex[0].pos;
	vec3 v1 = tr.vertex[1].pos;
	vec3 v2 = tr.vertex[2].pos;
	vec3 normal = cross(v1-v0, v2-v0);

	if(abs(dot(normal, ray_dir)) < EPSILON)
		return false;

	float d = dot(normal, v0);
	t = (dot(normal, ray_org) + d)/dot(normal, ray_dir);
	if(t > 0)
		return false;
	out_point = ray_org + t*ray_dir;

	vec3 cross1 = cross(v1-v0, out_point - v0);
	vec3 cross2 = cross(v2-v1, out_point - v1);
	vec3 cross3 = cross(v2-v0, out_point - v2);
	if(dot(normal, cross1) < 0)
		return false;
	if(dot(normal, cross2) < 0)
		return false;
	if(dot(normal, cross3) < 0)
		return false;
	
	return true;
}


int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {

		SDL_Window* window = SDL_CreateWindow("SDL2 ImGui Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if ( true ) {
            SDL_bool done = SDL_FALSE;
			SDL_SetRelativeMouseMode(SDL_FALSE);
            
			std::vector<Obj> objects;
            objects.push_back( Obj("./monkey_smooth.obj") );

			ImGui::CreateContext();
			ImGuiSDL::Initialize(renderer, WIDTH, HEIGHT);

            bool mouse_down = false;
            camera cam(vec3(0, 2, 3), vec3(0, 0, -1), vec3(0, 1, 0), 90.0f, 1.5f, WIDTH, HEIGHT);

			// g++ sdl_test.cpp -IC:\mingw64\include -LC:\mingw64\lib -g -O3 -w -lmingw32 -lSDL2main -lSDL2 -o tst.exe
			double ms = -1;
			char buf[256];

			float my_color[4];
			bool my_tool_active;
            while (!done) {
                SDL_Event event;
				ImGuiIO& io = ImGui::GetIO();
                std::clock_t then = std::clock();

				int mouseX, mouseY;
				const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

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
						if (ImGui::MenuItem("Open..", "Ctrl+O")) { objects.push_back( Obj(buf) ); }
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
				ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
				ImGui::BeginChild("Scrolling");
				ImGui::Text("FPS: %d", (int)ms);
				ImGui::Text("Cam Rotation: (%f, %f, %f)", cam.rotation.x(), cam.rotation.y(), cam.rotation.z());
				ImGui::Text("Cam Position: (%f, %f, %f)", cam._from.x(), cam._from.y(), cam._from.z());
				ImGui::Text("Color: (%f, %f, %f)", my_color[0]*255.0, my_color[1]*255.0, my_color[2]*255.0, my_color[3]*255.0);
				ImGui::EndChild();
				ImGui::End();

				SDL_SetRenderDrawColor(renderer, my_color[0]*255, my_color[1]*255, my_color[2]*255, my_color[3]*255);
				SDL_RenderClear(renderer);
                cam.render_scene(objects, renderer);
				ImGui::Render();
				ImGuiSDL::Render(ImGui::GetDrawData());
                SDL_RenderPresent(renderer);

				std::clock_t now = std::clock();
                ms = (double)(now - then);
				ms /= CLOCKS_PER_SEC;
				// objects[0].rot_x(4.f * ms);
				// objects[0].rot_y(-4.f * ms);

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
							SDL_SetRelativeMouseMode(SDL_TRUE);
                        }
						if( event.button.button == SDL_BUTTON_LEFT){

							int x, y;
							SDL_GetMouseState(&x, &y);
							printf("x = %d| y = %d\n", x, y);
							float invWidth = 1.0f/float(WIDTH), invHeight = 1.0f/float(HEIGHT); 
							float aspectratio = WIDTH/float(HEIGHT); 
							//Angulo de abertura da camera
							float angle = tan((cam.fov * 0.5f) * (M_PI / 180.0f)) * cam._near ; //Multiplica pelo near (zoom)
							float half_width = angle * aspectratio;
							float half_height = angle;
							float Px = -(2.0 * ( ( (float)x ) * invWidth) - 1.0) * cam.right*cam._near; 
							float Py = (1.0 - 2.0 * ( ( (float)y ) * invHeight)) * cam.top*cam._near; 

							vec3 dir = vec3(Px, Py, -1);
							cam.camToWorld.mult_vec_matrix(dir, dir);
							dir.make_unit_vector();

							printf(" dir = (%f, %f, %f)\n", dir.x(), dir.y(), dir.z());
							vec3 col, point;
							float t;

							bool finish = false;
							for( auto &object : objects){
								printf("iterating...\n");
								for( auto &tr : object.mesh.tris){
									if( intersects_triangle(tr, cam._from, dir, col, point, t) ){
										printf("pressed left mouse\n");
										object.col = vec3(0, 255, 0);
										finish = true;
										break;
									}
								}
								if( finish )
									break;
							}
						}
                    }
                    if( event.type == SDL_MOUSEBUTTONUP )
                    {
                        //If the left mouse button was released
                        if( event.button.button == SDL_BUTTON_RIGHT )
                        { 
                            mouse_down = false;
							SDL_SetRelativeMouseMode(SDL_FALSE);
                        }
                    }

                    if( event.type == SDL_MOUSEMOTION && mouse_down ){
                        float x = event.motion.xrel;
                        float y = event.motion.yrel;

						cam.rotate(-x * 15.f * ms, -y * 15.f * ms);
                    }

                    if (event.type == SDL_QUIT)
                        done = SDL_TRUE;
					
                }
                ms = 1.0/ms;
				// printf("frametime: %f\n", ms);
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
