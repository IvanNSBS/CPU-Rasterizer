#pragma once

#include <vector>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include "vec3.h"
#include "vec2f.h"
#include "matrix44.h"

#define min_x 0
#define max_x 1
#define min_y 2
#define max_y 3
#define min_z 4
#define max_z 5

struct Triangle {
	vec3 p[3];
	vec3 n;
};

class Mesh 
{
public:
	std::vector<vec3> vertices;
	std::vector<vec2f> uvs;
	std::vector<vec3> normals;
	float bounding_box[6];
	vec3 bbox_center;

	Mesh() {
		bounding_box[min_x] = 999999.0f;
		bounding_box[max_x] = -999999.0f;
		bounding_box[min_y] = 999999.0f;
		bounding_box[max_y] = -999999.0f;
		bounding_box[min_z] = 999999.0f;
		bounding_box[max_z] = -999999.0f;
	}
	~Mesh() {}

	bool load_mesh_from_file(const char* path) 
	{
		vertices.clear();
		uvs.clear();
		normals.clear();

		std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
		std::vector< vec3 > temp_vertices;
		std::vector< vec2f > temp_uvs;
		std::vector< vec3 > temp_normals;

		std::ifstream f(path);
		if (!f.is_open())
		{
			std::cout << "File cannot be oppened or does not exist\n";
			return false;
		}

		std::cout << "file was  oppened!\n";

		
		while (!f.eof())
		{
			char line[256];
			f.getline(line, 256);

			std::stringstream s;
			s << line;

			char junk;

			if ( line[0] == 'v')
			{
				if (line[1] == 't') 
				{
					vec2f uv;
					s >> junk >> junk >> uv[0] >> uv[1];
					temp_uvs.push_back(uv);
				}
				if (line[1] == 'n') 
				{
					vec3 normal;
					s >> junk >> junk >> normal[0] >> normal[1] >> normal[2];
					temp_normals.push_back(normal);
				}
				else {
					vec3 vertex;
					s >> junk >> vertex[0] >> vertex[1] >> vertex[2];
					temp_vertices.push_back(vertex);
				}
			}

			else if ( line[0] == 'f')
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

				s >> junk >> vertex1 >> vertex2 >> vertex3;
				int fstslash = vertex1.find("/");
				int sndslash = vertex1.find("/", fstslash+1);
				int trdslash = vertex1.find("/", sndslash+1);
				std::string fst = vertex1.substr(0, fstslash);
				std::string snd = vertex1.substr(fstslash+1, sndslash - fstslash - 1);
				std::string trd = vertex1.substr(sndslash+1);
				vertexIndex[0] = atoi( fst.c_str() );
				uvIndex[0] = atoi(snd.c_str());
				normalIndex[0] = atoi( trd.c_str() );

				fstslash = vertex2.find("/");
				sndslash = vertex2.find("/", fstslash + 1);
				trdslash = vertex2.find("/", sndslash + 1);
				fst = vertex2.substr(0, fstslash);
				snd = vertex2.substr(fstslash + 1, sndslash - fstslash - 1);
				trd = vertex2.substr(sndslash + 1);
				vertexIndex[1] = atoi(fst.c_str());
				uvIndex[1] = atoi(snd.c_str());
				normalIndex[1] = atoi(trd.c_str());

				fstslash = vertex3.find("/");
				sndslash = vertex3.find("/", fstslash + 1);
				trdslash = vertex3.find("/", sndslash + 1);
				fst = vertex3.substr(0, fstslash);
				snd = vertex3.substr(fstslash + 1, sndslash - fstslash - 1);
				trd = vertex3.substr(sndslash + 1);
				vertexIndex[2] = atoi(fst.c_str());
				uvIndex[2] = atoi(snd.c_str());
				normalIndex[2] = atoi(trd.c_str());


				
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
		}
		

		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			unsigned int vertexIndex = vertexIndices[i];
			vec3 vertex = temp_vertices[vertexIndex - 1];
			this->vertices.push_back(vertex);

			if (vertex.x() < bounding_box[min_x])
				bounding_box[min_x] = vertex.x();
			if (vertex.x() > bounding_box[max_x])
				bounding_box[max_x] = vertex.x();

			if (vertex.y() < bounding_box[min_y])
				bounding_box[min_y] = vertex.y();
			if (vertex.y() > bounding_box[max_y])
				bounding_box[max_y] = vertex.y();

			if (vertex.z() < bounding_box[min_z])
				bounding_box[min_z] = vertex.z();
			if (vertex.z() > bounding_box[max_z])
				bounding_box[max_z] = vertex.z();
		}

		bbox_center = vec3( 
			(bounding_box[max_x] + bounding_box[min_x]) / 2.0f, 
			(bounding_box[max_y] + bounding_box[min_y]) / 2.0f, 
			(bounding_box[max_z] + bounding_box[min_z]) / 2.0f);

		for (unsigned int i = 0; i < uvIndices.size(); i++)
		{
			unsigned int uvIndex = uvIndices[i];
			vec2f uv = temp_uvs[uvIndex - 1];
			this->uvs.push_back(uv);
		}

		for (unsigned int i = 0; i < normalIndices.size(); i++)
		{
			unsigned int normalIndex = normalIndices[i];
			vec3 nrml = temp_normals[normalIndex - 1];
			this->normals.push_back(nrml);
		}

		std::cout << "vertSize = " << vertexIndices.size();
		std::cout << "normalSize = " << normalIndices.size();

		return true;
	}
};

struct Transform 
{
	matrix44 transf;
	vec3 scale;
	vec3 rotation;
	vec3 location;
};

class object 
{
public:
	Mesh mesh;
	Transform transform;
};