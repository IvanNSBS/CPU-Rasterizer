#include <math.h>
#include "Graphics.h"
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include "camera.h"
#include <chrono>
#include <algorithm>
#include "object.h"

#define MIN 0
#define MAX 1
#define PI 3.14159265


using namespace std;

static const int WIDTH = 1000;
static const int HEIGHT = 600;

static vector<vec3> cubepts = {
	vec3(1, 0, 0), vec3(1,1,0), vec3(0,1,0), // frontfirst tri
	vec3(0,1,0), vec3(0,0,0), vec3(1,0,0), // front second tri
	vec3(0,0,0), vec3(0,1,0), vec3(0,1,1), // right 1st tri
	vec3(0,1,1), vec3(0,0,1), vec3(0,0,0), //right 2nd tri
	vec3(0,0,1), vec3(0,1,1), vec3(1,1,1), // back 1st tri
	vec3(1,1,1), vec3(1,0,1), vec3(0,0,1), // back 2nd tri
	vec3(1,1,1), vec3(1,1,0), vec3(1,0,1),//side 1st tri
	vec3(1,0,1), vec3(1,0,0), vec3(1,1,0), //side 2nd tri
	vec3(1,1,0), vec3(1,1,1), vec3(0,1,1), //top 1st tri
	vec3(0,1,1), vec3(0,1,0), vec3(1,1,0), // top 2nd tri
	vec3(1,0,0), vec3(1,0,1), vec3(0,0,1),
	vec3(0,0,1), vec3(0,0,0), vec3(1,0,0),
};


void render_scene( Mesh mesh, camera &cam, sf::RenderWindow &wind ) {

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
			float z1 = (t1.p[0].z() + t1.p[1].z(), t1.p[2].z()) / 3.0f;
			float z2 = (t2.p[0].z() + t2.p[1].z(), t2.p[2].z()) / 3.0f;
			return z1 < z2;
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
				sf::Vector2f p1(praster1.x(), praster1.y());
				sf::Vector2f p2(praster2.x(), praster2.y());
				sf::Vector2f p3(praster3.x(), praster3.y());

				vec3 col(500, 500, 500);
				col *= dp;

				sf::ConvexShape convex;
				convex.setFillColor(sf::Color(col.x(), col.y(), col.z(), 255));
				// resize it to 5 points
				convex.setPointCount(3);

				// define the points
				convex.setPoint(0, p1);
				convex.setPoint(1, p2);
				convex.setPoint(2, p3);

				wind.draw(convex);

				wind.draw(LineShape(p1, p2, sf::Color(255, 255, 255, 255)));
				wind.draw(LineShape(p1, p3, sf::Color(255, 255, 255, 255)));
				wind.draw(LineShape(p2, p3, sf::Color(255, 255, 255, 255)));
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


int main()
{
	
	Mesh monkey_mesh;
	if (!monkey_mesh.load_mesh_from_file("./cube.obj"))
		std::cout << "monkey wasnt loaded\n";

	sf::ContextSettings settings;
	settings.antialiasingLevel = 0;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), 
							"Rasterizer", 
							sf::Style::Titlebar | sf::Style::Close, settings);

	std::vector<LineShape> poly_lines;	// linhas da malha poligonal
	camera cam(vec3(0, 0, 300), vec3(0, 0, -1), vec3(0, 1, 0), 60.0f, 0.1f, WIDTH, HEIGHT);

	sf::Font font;
	if (font.loadFromFile("../arial_narrow_7.ttf"))
		std::cout << "Font was loaded!\n";
	else
		std::cout << "Font wasn't loaded...\n";

	sf::Text base_text;
	base_text.setFont(font);
	base_text.setStyle(sf::Text::Regular);
	base_text.setCharacterSize(30);
	base_text.setFillColor(sf::Color(255, 255, 255));

	base_text.setPosition(0, 0);
	base_text.setString("FPS: 20");

	sf::Vector2i mouse_pos;
	vec2f cur_pos;
	vec2f old_pos;
	bool holding_click = false;

	sf::Clock clock;
	sf::Time time;

	float yaw = 0.0f;
	float pitch = 0.0f;

	while (window.isOpen())
	{
		clock.restart().asSeconds();
		sf::Event event;
		while (window.pollEvent(event))
		{
			mouse_pos = sf::Mouse::getPosition(window);

			if (holding_click) {
				cur_pos = vec2f((float)mouse_pos.x, (float)mouse_pos.y);
				vec2f sub = cur_pos - old_pos;

				sub[0] /= WIDTH;
				sub[1] /= HEIGHT;

				yaw = sub.x() * 60.f;
				pitch = -sub.y() * 60.f ;

				rot_x(pitch, monkey_mesh, monkey_mesh.bbox_center);
				rot_y(-yaw, monkey_mesh, monkey_mesh.bbox_center);
				old_pos = cur_pos;
			}

			if (event.type == sf::Event::Closed)
				window.close();

			//add control point
			else if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{
				old_pos = vec2f((float)mouse_pos.x, (float)mouse_pos.y);
				holding_click = true;
			}
			else if (event.type == event.MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
			{
				holding_click = false;
			}

			else if (sf::Keyboard::isKeyPressed){

				vec3 dir (0.0f, 0.0f, 0.0f);

				if (event.key.code == sf::Keyboard::W)
					dir +=(vec3(0.f, 0.2f, 0.0f));
				if (event.key.code == sf::Keyboard::S)
					dir += (vec3(0.0f, -0.2f, 0.0f));
				if (event.key.code == sf::Keyboard::A)
					dir += ( vec3(-0.20f, 0.0f, 0.0f));
				if (event.key.code == sf::Keyboard::D)
					dir += (vec3(0.20f, 0.0f, 0.0f));
				if (event.key.code == sf::Keyboard::Q)
					dir += (vec3(0.f, 0.f, -1.8f));
				if (event.key.code == sf::Keyboard::E)
					dir += (vec3(0.0f, 0.0f, 1.8f));
				
				cam.move(dir);

			}
		}


		//rot_x(-0.01f, monkey_mesh, monkey_mesh.bbox_center);
		//rot_y(-0.04f, monkey_mesh, monkey_mesh.bbox_center);
		//scale(1.0001f, 1.0001f, 1.0001f, cubepts);
		//translate( vec3(0.0f, 0.0001f, 0.1f), monkey_mesh.vertices);
		render_scene(monkey_mesh, cam, window);

		time = clock.getElapsedTime();
		float fps = 1.0f / time.asSeconds();
		clock.restart().asSeconds();

		//std::cout << fps << "\n";
		base_text.setString("FPS: " + std::to_string(fps) );


		window.draw(base_text);
		window.display();

		poly_lines.clear();
		window.clear(sf::Color(255,0,0));
	}

	return 0;
}
