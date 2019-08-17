
#include <string>
#include <math.h>
#include "camera.h"
#include <chrono>
#include "object.h"

#include "ImGUI/imgui_sdl.h"
#include "ImGUI/imgui.h"

#ifdef _WIN32 || WIN32
	#include <SDL.h>
#elif defined(__unix__)
	#include <SDL2/SDL.h>
#endif

#define MIN 0
#define MAX 1
#define PI 3.14159265

static const int WIDTH = 800;
static const int HEIGHT = 600;

bool intersects_triangle( Triangle &tr, vec3 ray_org, vec3 ray_dir, vec3 &out_col, vec3 &out_point, float &t)
{
    const float EPSILON = 0.0000001;
    vec3 vertex0 = tr.vertex[0].pos;
    vec3 vertex1 = tr.vertex[1].pos;  
    vec3 vertex2 = tr.vertex[2].pos;
    vec3 edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = cross(ray_dir, edge2);
    a = dot( edge1, h );
    if (a > -EPSILON && a < EPSILON){
        return false;    // This ray is parallel to this triangle.
	}
    f = 1.0/a;
    s = ray_org - vertex0;
    u = f *dot(s,h);
    if (u < 0.0 || u > 1.0)
        return false;
    q = cross(s, edge1);
    v = f * dot(ray_dir, q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    // At this stage we can compute t to find out where the intersection point is on the line.
    t = f * dot(edge2, q);
    if (t > EPSILON) // ray intersection
    {
		float w = 1.0 - u - v;
		vec3 normal = w*tr.vertex[0].normal + u*tr.vertex[1].normal + v*tr.vertex[1].normal; 
		out_col = vec3(0, 255, 0) * (dot(normal, vec3(0,0, -1.0f)));
        out_point = ray_org + ray_dir * t;
        return true;
    }
    else {
        return false;
	}// This means that there is a line intersection but not a ray intersection.
}

void fill_triangle( const Triangle &tr, SDL_Renderer *renderer, const vec3 &col)
{

}

void draw_lines( const vec2 &p0, const vec2 &p1, SDL_Renderer *renderer, const vec3 &col)
{

	// // calculate dx , dy
	// int dx = p1.x() - p0.x();
	// int dy = p1.y() - p0.y();

	// // Depending upon absolute value of dx & dy
	// // choose number of steps to put pixel as
	// // steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy)
	// int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	// // calculate increment in x & y for each steps
	// float Xinc = dx / (float) steps;
	// float Yinc = dy / (float) steps;

	// // Put pixel for each step
	// float X = p0.x();
	// float Y = p0.y();
	// for (int i = 0; i <= steps; i++)
	// {
	// 	SDL_RenderDrawPoint(renderer, X, Y);
	// 	X += Xinc;
	// 	Y += Yinc;
	// }

	int dx = p1.x() - p0.x();
	int dy = p1.y() - p0.y();
	// points are too close, draw point and return
	if( dx == 0 && dy == 0){
		SDL_RenderDrawPoint(renderer, p1.x(), p1.y());
		return;
	}

	vec2 origin = p0;
	vec2 line = unit_vector(p1 - p0);
	float end, start, step;
	if( dx != 0 ){
		end = p1.x() > p0.x() ? p1.x() : p0.x();
		start = p1.x() > p0.x() ? p0.x() : p1.x();
		step = abs(line.x());
	}
	else{
		end = p1.y() > p0.y() ? p1.y() : p0.y();
		start = p1.y() > p0.y() ? p0.y() : p1.y();
		step = abs(line.y());	
	}

	// printf("start = %f | end = %f | step = %f\n", start, end, step);

	for(float i = start; i <= end; i+=step){
		SDL_RenderDrawPoint(renderer, origin.x(), origin.y());
		origin += line;
	}
}

void render_scene( std::vector<Obj> objs, camera &cam, SDL_Window *wind, SDL_Renderer* renderer) {

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

			vec3 ray = obj.mesh.tris[i].vertex[0].pos - cam._from;
			if ( dot(normal, ray ) < 0.0f ) {

				float dp = dot(normal, light);
				vec2 praster1;
				vec2 praster2;
				vec2 praster3;

				vec3 col(255, 255, 255);
				SDL_SetRenderDrawColor(renderer, obj.col.x(), obj.col.y(), obj.col.z(), SDL_ALPHA_OPAQUE);

				bool v1, v2, v3;
				v1 = cam.compute_pixel_coordinates(obj.mesh.tris[i].vertex[0].pos, praster1);
				v2 = cam.compute_pixel_coordinates(obj.mesh.tris[i].vertex[1].pos, praster2);
				v3 = cam.compute_pixel_coordinates(obj.mesh.tris[i].vertex[2].pos, praster3);

				if(v1 && v2)
					draw_lines(praster1, praster2, renderer, col);
				if(v1 && v3)
					draw_lines(praster1, praster3, renderer, col);
				if(v2 && v3)
					draw_lines(praster2, praster3, renderer, col);

				// if(v1 && v2)
				// 	SDL_RenderDrawLine(renderer, praster1[0], praster1[1], praster2[0], praster2[1]);
				// if(v1 && v3)
				// 	SDL_RenderDrawLine(renderer, praster1[0], praster1[1], praster3[0], praster3[1]);
				// if(v2 && v3)
				// 	SDL_RenderDrawLine(renderer, praster2[0], praster2[1], praster3[0], praster3[1]);
			}
		}
	}
}



int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {

		SDL_Window* window = SDL_CreateWindow("SDL2 ImGui Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if ( true ) {
            // SDL_SetWindowTitle(window, "Test Title!!");
            SDL_bool done = SDL_FALSE;
			SDL_SetRelativeMouseMode(SDL_FALSE);
            
			std::vector<Obj> objects;
            objects.push_back( Obj("./monkey_smooth.obj") );
			objects[0].rot_x(34.6);
			objects[0].rot_y(-50.f);
			objects[0].rot_z(-4.56f);

			ImGui::CreateContext();
			ImGuiSDL::Initialize(renderer, WIDTH, HEIGHT);

            bool mouse_down = false;
            camera cam(vec3(0, 0, 20), vec3(0, 0, -1), vec3(0, 1, 0), 60.0f, 1.f, WIDTH, HEIGHT);

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
						if (ImGui::MenuItem("Open..", "Ctrl+O")) { objects.push_back( Obj("./monkey_smooth.obj") ); objects[objects.size()-1].translate( vec3(10, 0, 0) ); }
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
				ImGui::Text("FPS: %d", (int)ms);
				ImGui::BeginChild("Scrolling");
				ImGui::Text("Cam Rotation: (%f, %f, %f)", cam.rotation.x(), cam.rotation.y(), cam.rotation.z());
				ImGui::Text("Cam Position: (%f, %f, %f)", cam._from.x(), cam._from.y(), cam._from.z());
				ImGui::Text("Color: (%f, %f, %f)", my_color[0]*255.0, my_color[1]*255.0, my_color[2]*255.0, my_color[3]*255.0);
				ImGui::EndChild();
				ImGui::End();

				SDL_SetRenderDrawColor(renderer, my_color[0]*255, my_color[1]*255, my_color[2]*255, my_color[3]*255);
				SDL_RenderClear(renderer);


                render_scene(objects, cam, window, renderer);
				ImGui::Render();
				ImGuiSDL::Render(ImGui::GetDrawData());
                SDL_RenderPresent(renderer);

				std::clock_t now = std::clock();
                ms = (double)(now - then);
				ms /= CLOCKS_PER_SEC;
				objects[0].rot_x(4.f * ms);
				objects[0].rot_y(-4.f * ms);

                while (SDL_PollEvent(&event)) {

					float vertical_speed = 1.0f;	
					float up_speed = 0.2f;
					float horizontal_speed = 0.10f;
					if( event.type == SDL_KEYDOWN){
						if( event.key.keysym.sym == SDLK_a )
							cam.move( vec3(-horizontal_speed, 0.0f, 0.0f) );
						if( event.key.keysym.sym == SDLK_d )
							cam.move( vec3(horizontal_speed, 0.0f, 0.0f) );
						if( event.key.keysym.sym == SDLK_w )
							cam.move( vec3(0.0f, 0.0f, -vertical_speed) );
						if( event.key.keysym.sym == SDLK_s )
							cam.move( vec3(0.0f, 0.0f, vertical_speed) );

						if( event.key.keysym.sym == SDLK_q )
							cam.move( vec3(0.00f, -up_speed, 0.0f) );
						if( event.key.keysym.sym == SDLK_e )
							cam.move( vec3(0.0f, up_speed, 0.0f) );

						
						if( event.key.keysym.sym == SDLK_z )
							cam.rot_x(450.f * ms);
						if( event.key.keysym.sym == SDLK_x )
							cam.rot_y(450.f * ms);

						if( event.key.keysym.sym == SDLK_c )
							cam.rot_x(-450.f * ms);
						if( event.key.keysym.sym == SDLK_v )
							cam.rot_y(-450.f * ms);
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
							float Px = (2.0 * ( ( (float)x ) * invWidth) - 1.0) * half_width; 
							float Py = (1.0 - 2.0 * ( ( (float)y ) * invHeight)) * half_height; 

							vec3 dir = vec3(Px, Py, -1);
							cam.camToWorld.mult_vec_matrix(dir, dir);
							dir.make_unit_vector();
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

						cam.rotate(x * 15.f * ms, -y * 15.f * ms);
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
