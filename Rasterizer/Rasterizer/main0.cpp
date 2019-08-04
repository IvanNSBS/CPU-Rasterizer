#include <math.h>
#include "Graphics.h"
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include "camera.h"
#include "mesh.h"
#include <chrono>

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


void render_scene( std::vector<vec2f> &raster_pts, std::vector<vec3> &world_pts, camera &cam ) {

	raster_pts.clear();

	for (int i = 0; i < world_pts.size(); i += 3)
	{
		vec2f praster1;
		vec2f praster2;
		vec2f praster3;

		cam.compute_pixel_coordinates(world_pts[i + 0], praster1);
		cam.compute_pixel_coordinates(world_pts[i + 1], praster2);
		cam.compute_pixel_coordinates(world_pts[i + 2], praster3);

		raster_pts.push_back(praster1);
		raster_pts.push_back(praster2);
		raster_pts.push_back(praster3);
	}
}

void scale(float s_x, float s_y, float s_z, std::vector<vec3> &points) {

	matrix44 tr(s_x, 0, 0, 0,
				0, s_y, 0, 0,
				0, 0, s_z, 0,
				0, 0 , 0, 1);
	for (int i = 0; i < points.size(); ++i) {
		tr.multVecMatrix(points[i], points[i]);
	}
}

void translate(vec3 tl, std::vector<vec3> &points) {

	matrix44 tr(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				tl.x(), tl.y(), tl.z(), 1);

	for (int i = 0; i < points.size(); ++i) {
		tr.multVecMatrix(points[i], points[i]);
	}
}

void rot_y(float deg, std::vector<vec3> &points, vec3 rot_point) {

	matrix44 tr(		1,			  0,			0,			0,
						0,			  1,			0,			0,
						0,			  0,			1,			0,
			    -rot_point.x(), -rot_point.y(), -rot_point.z(), 1);
	matrix44 itr = tr.inverse();
	float sen = sin(deg*PI / 180.0);
	float co = cos(deg*PI / 180);
	matrix44 rot(co, 0, sen, 0,
		0, 1, 0, 0,
		-sen, 0, co, 0,
		0, 0, 0, 1);
	matrix44 result = (tr*rot)*itr;

	for (int i = 0; i < points.size(); ++i) {
		result.multVecMatrix(points[i], points[i]);
	}
}

void rot_x(float deg, std::vector<vec3> &points, vec3 rot_point) {

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

	for (int i = 0; i < points.size(); ++i) {
		result.multVecMatrix(points[i], points[i]);
	}
}

int main()
{
	
	mesh monkey_mesh;
	if (!monkey_mesh.load_mesh_from_file("./monkey.obj"))
		std::cout << "monkey wasnt loaded\n";

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), 
							"Rasterizer", 
							sf::Style::Titlebar | sf::Style::Close, settings);

	std::vector<vec2f> poly_pts;
	std::vector<LineShape> poly_lines;	// linhas da malha poligonal
	camera cam(vec3(20, 0, 300), vec3(0, 0, -1), vec3(0, 1, 0), 60.0f, 0.1f, WIDTH, HEIGHT);

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

	sf::Clock clock;
	sf::Time time;


	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			mouse_pos = sf::Mouse::getPosition(window);

			if (event.type == sf::Event::Closed)
				window.close();

			//add control point
			else if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{

				sf::Vector2f localPosition((float)mouse_pos.x, (float)mouse_pos.y);

			}

			//remove control point
			else if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
			{

			}

			else if(event.type == event.MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) 
			{

			}
		}


		//move control point
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			
		}
		
		rot_x(1.5f, monkey_mesh.vertices, monkey_mesh.vertices[0]);
		rot_y(-0.9f, monkey_mesh.vertices, monkey_mesh.vertices[0]);
		//scale(1.0001f, 1.0001f, 1.0001f, cubepts);
		//translate( vec3(0.0f, 0.000f, 0.01f), cubepts);
		render_scene(poly_pts, monkey_mesh.vertices, cam);

		//control polygon
		for (int i = 0; i < monkey_mesh.vertices.size(); i += 3)
		{
			sf::Vector2f p1(poly_pts[i].x(), poly_pts[i].y());
			sf::Vector2f p2(poly_pts[i + 1].x(), poly_pts[i + 1].y());
			sf::Vector2f p3(poly_pts[i + 2].x(), poly_pts[i + 2].y());
			poly_lines.push_back(LineShape(p1, p2, sf::Color(255, 255, 255, 255)));
			poly_lines.push_back(LineShape(p1, p3, sf::Color(255, 255, 255, 255)));
			poly_lines.push_back(LineShape(p2, p3, sf::Color(255, 255, 255, 255)));
		}
	
		for (int i = 0; i < poly_lines.size(); ++i)
			window.draw(poly_lines[i]);

		time = clock.getElapsedTime();
		float fps = 1.0f / time.asSeconds();
		clock.restart().asSeconds();
		std::cout << fps << "\n";
		base_text.setString("FPS: " + std::to_string(fps) );


		window.draw(base_text);
		window.display();

		poly_lines.clear();
		window.clear(sf::Color(0,0,0));
	}

	return 0;
}
