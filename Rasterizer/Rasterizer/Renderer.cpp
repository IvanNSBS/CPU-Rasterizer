#include "camera.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

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

void renderScene(){


    camera cam(vec3(10,80,200), vec3(0,0,-1), vec3(0,1,0), 80.0f, 0.1f, 320, 240);

    std::ofstream ofs; 
    ofs.open("./tst.svg"); 
    ofs << "<svg version=\"1.1\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.w3.org/2000/svg\" width=\"" << cam.imgWidth << "\" height=\"" << cam.imgHeight << "\">" << std::endl;
    
    ofs << "    <rect width=\"" << cam.imgWidth << "\" height=\"" << cam.imgHeight << "\" stroke=\"black\" stroke-width=\"0\" fill=\"rgb(150,150,150)\"/>\n" ; 
    
    for(int i = 0; i < cubepts.size(); i+=3)
    {
        vec2f praster1;
        vec2f praster2;
        vec2f praster3; 

        cam.compute_pixel_coordinates(cubepts[i+0], praster1);
        cam.compute_pixel_coordinates(cubepts[i+1], praster2);
        cam.compute_pixel_coordinates(cubepts[i+2], praster3);

        cout << "from " << cubepts[i+0] << " to at " << i   << " " << praster1 << "\n";
        cout << "from " << cubepts[i+1] << " to at " << i+1 << " " << praster2 << "\n";
        cout << "from " << cubepts[i+2] << " to at " << i+2 << " " << praster3 << "\n";

        ofs << "    <rect x=\"" << praster1.x()-2 << "\" y=\"" << praster1.y()-2 << "\" width=\"4\" height=\"4\" style=\"fill:white;stroke:pink;stroke-width:0;opacity:1\" />\n";
        ofs << "    <rect x=\"" << praster2.x()-2 << "\" y=\"" << praster2.y()-2 << "\" width=\"4\" height=\"4\" style=\"fill:white;stroke:pink;stroke-width:0;opacity:1\" />\n";
        ofs << "    <rect x=\"" << praster3.x()-2 << "\" y=\"" << praster3.y()-2 << "\" width=\"4\" height=\"4\" style=\"fill:white;stroke:pink;stroke-width:0;opacity:1\" />\n";

        ofs << "    <line x1=\"" << praster1.x() << "\" y1=\"" << praster1.y() << "\" x2=\"" << praster2.x() << "\" y2=\"" << praster2.y() << "\" style=\"stroke:rgb(255,255,255);stroke-width:1.2\" />\n"; 
        ofs << "    <line x1=\"" << praster1.x() << "\" y1=\"" << praster1.y() << "\" x2=\"" << praster3.x() << "\" y2=\"" << praster3.y() << "\" style=\"stroke:rgb(255,255,255);stroke-width:1.2\" />\n"; 
        ofs << "    <line x1=\"" << praster2.x() << "\" y1=\"" << praster2.y() << "\" x2=\"" << praster3.x() << "\" y2=\"" << praster3.y() << "\" style=\"stroke:rgb(255,255,255);stroke-width:1.2\" />\n"; 
    }

    ofs << "</svg>";
}