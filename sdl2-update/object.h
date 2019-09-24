#include <vector>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include "vec3.h"
#include "vec2.h"
#include "matrix44.h"

#define min_x 0
#define max_x 1
#define min_y 2
#define max_y 3
#define min_z 4
#define max_z 5

#define M_PI 3.141592653589793

struct Vertex{
	vec3 pos;
	vec3 normal;
	vec2 uv;
};


struct Triangle {

	Vertex vertex[3];

	Triangle( const std::vector<vec3> v, const std::vector<vec2> t, const std::vector<vec3> n)
	{
		vertex[0].pos = v[0];
		vertex[1].pos = v[1];
		vertex[2].pos = v[2];
		vertex[0].uv = t[0];
		vertex[1].uv = t[1];
		vertex[2].uv = t[2];
		vertex[0].normal = n[0];
		vertex[1].normal = n[1];
		vertex[2].normal = n[2];
	}

	~Triangle(){}
};

class Mesh 
{
public:
	std::vector<Triangle> tris;
	vec3 bbox_center;

	Mesh() {}
	~Mesh() {}

	bool load_mesh_from_file(const char* path) 
	{
		tris.clear();

		float bounding_box[6];
		bounding_box[min_x] = 999999.0f;
		bounding_box[max_x] = -999999.0f;
		bounding_box[min_y] = 999999.0f;
		bounding_box[max_y] = -999999.0f;
		bounding_box[min_z] = 999999.0f;
		bounding_box[max_z] = -999999.0f;

		std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
		std::vector< vec3 > temp_vertices;
		std::vector< vec2 > temp_uvs;
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
					vec2 uv;
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
		
		auto update_bbox = [&](std::vector<vec3> verts){
			for(vec3 v : verts){
			if (v.x() < bounding_box[min_x])
				bounding_box[min_x] = v.x();
			if (v.x() > bounding_box[max_x])
				bounding_box[max_x] = v.x();

			if (v.y() < bounding_box[min_y])
				bounding_box[min_y] = v.y();
			if (v.y() > bounding_box[max_y])
				bounding_box[max_y] = v.y();

			if (v.z() < bounding_box[min_z])
				bounding_box[min_z] = v.z();
			if (v.z() > bounding_box[max_z])
				bounding_box[max_z] = v.z();
			}
		};

		// for (unsigned int i = 0; i < 1; i+=3)
		for (unsigned int i = 0; i < vertexIndices.size(); i+=3)
		{
			unsigned int v1 = vertexIndices[i];
			unsigned int v2 = vertexIndices[i+1];
			unsigned int v3 = vertexIndices[i+2];

			unsigned int n1 = normalIndices[i];
			unsigned int n2 = normalIndices[i+1];
			unsigned int n3 = normalIndices[i+2];

			unsigned int uv1 = uvIndices[i];
			unsigned int uv2 = uvIndices[i+1];
			unsigned int uv3 = uvIndices[i+2];

			std::vector<vec3> vertices;
			vertices.push_back(temp_vertices[v1 - 1]);
			vertices.push_back(temp_vertices[v2 - 1]);
			vertices.push_back(temp_vertices[v3 - 1]);

			std::vector<vec2> uvs;
			if( uvIndices.size() > 0 ){
				uvs.push_back(temp_uvs[uv1 - 1]);
				uvs.push_back(temp_uvs[uv2 - 1]);
				uvs.push_back(temp_uvs[uv3 - 1]);
			}

			std::vector<vec3> normals;
			if( normalIndices.size() > 0){

				normals.push_back(temp_normals[n1 - 1]);
				normals.push_back(temp_normals[n2 - 1]);
				normals.push_back(temp_normals[n3 - 1]);
			}

			Triangle t(vertices, uvs, normals);
			tris.push_back(t);

			update_bbox(vertices);
		}

		bbox_center = vec3( 
			(bounding_box[max_x] + bounding_box[min_x]) / 2.0f, 
			(bounding_box[max_y] + bounding_box[min_y]) / 2.0f, 
			(bounding_box[max_z] + bounding_box[min_z]) / 2.0f);

		std::cout << "vertSize = " << vertexIndices.size() << "\n";
		std::cout << "normalSize = " << normalIndices.size() << "\n";
		std::cout << "uvSize = " << uvIndices.size() << "\n";

		return true;
	}
};

struct Transform 
{
	vec3 scale = vec3(1, 1, 1);
	vec3 rotation = vec3(0, 0, 0);
	vec3 location = vec3(0, 0, 0);
};

class Obj 
{
public:
	Mesh mesh;
	Transform transform;
	vec3 col;

	Obj(){}
	Obj( const char* file_path){
		mesh.load_mesh_from_file(file_path); transform = Transform(); 
		col = vec3(255,255,255);
	}
	
	~Obj(){}

	void scale( const vec3 &t ) {

		matrix44 tr(t.x(), 0, 0, 0,
					0, t.y(), 0, 0,
					0, 0, t.z(), 0,
					0, 0, 0, 1);

		for ( Triangle &tri : mesh.tris) {
			tr.mult_point_matrix(tri.vertex[0].pos, tri.vertex[0].pos);
			tr.mult_point_matrix(tri.vertex[1].pos, tri.vertex[1].pos);
			tr.mult_point_matrix(tri.vertex[2].pos, tri.vertex[2].pos);
		}

		tr.mult_point_matrix(mesh.bbox_center, mesh.bbox_center);

	}

	void translate(vec3 tl) {

		matrix44 tr(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					tl.x(), tl.y(), tl.z(), 1);


		for ( Triangle &tri : mesh.tris) {
			tr.mult_point_matrix(tri.vertex[0].pos, tri.vertex[0].pos);
			tr.mult_point_matrix(tri.vertex[1].pos, tri.vertex[1].pos);
			tr.mult_point_matrix(tri.vertex[2].pos, tri.vertex[2].pos);
		}

		tr.mult_point_matrix(mesh.bbox_center, mesh.bbox_center);

	}

	
	void rot_x(float deg) {

		vec3 tr = (mesh.bbox_center.x(), mesh.bbox_center.y(), mesh.bbox_center.z());
		float sen = sin(deg*M_PI / 180.0);
		float co = cos(deg*M_PI / 180);
		matrix44 rot(1, 0, 0, 0,
					0, co, -sen, 0,
					0, sen, co, 0,
					-mesh.bbox_center.x(), -mesh.bbox_center.y(), -mesh.bbox_center.z(), 1);
		matrix44 result = rot;

		for ( Triangle &tri : mesh.tris) {
			result.mult_point_matrix(tri.vertex[0].pos, tri.vertex[0].pos);
			result.mult_point_matrix(tri.vertex[1].pos, tri.vertex[1].pos);
			result.mult_point_matrix(tri.vertex[2].pos, tri.vertex[2].pos);

			translate_triangle(tr, tri);

			result.mult_vec_matrix(tri.vertex[0].normal, tri.vertex[0].normal);
			result.mult_vec_matrix(tri.vertex[1].normal, tri.vertex[1].normal);
			result.mult_vec_matrix(tri.vertex[2].normal, tri.vertex[2].normal);

		}
		result.mult_point_matrix(mesh.bbox_center, mesh.bbox_center);
		translate_triangle(tr, mesh.bbox_center);
	}

	void translate_triangle(vec3 translation, Triangle &tri)
	{
		matrix44 tr(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					translation.x(), translation.y(), translation.z(), 1);

		tr.mult_point_matrix(tri.vertex[0].pos, tri.vertex[0].pos);
		tr.mult_point_matrix(tri.vertex[1].pos, tri.vertex[1].pos);
		tr.mult_point_matrix(tri.vertex[2].pos, tri.vertex[2].pos);
	}

	void translate_triangle(vec3 translation, vec3 &tri)
	{
		matrix44 tr(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					translation.x(), translation.y(), translation.z(), 1);

		tr.mult_point_matrix(tri, tri);
		tr.mult_point_matrix(tri, tri);
		tr.mult_point_matrix(tri, tri);
	}

	void rot_y(float deg) {

		float sen = sin(deg*M_PI / 180.0f);
		float co = cos(deg*M_PI / 180.0f);
		vec3 tr = (mesh.bbox_center.x(), mesh.bbox_center.y(), mesh.bbox_center.z());
		matrix44 rot(co, 0, sen, 0,
					0, 1, 0, 0,
					-sen, 0, co, 0,
					-mesh.bbox_center.x(), -mesh.bbox_center.y(), -mesh.bbox_center.z(), 1);
		matrix44 result = rot;

		for ( Triangle &tri : mesh.tris) {
			result.mult_point_matrix(tri.vertex[0].pos, tri.vertex[0].pos);
			result.mult_point_matrix(tri.vertex[1].pos, tri.vertex[1].pos);
			result.mult_point_matrix(tri.vertex[2].pos, tri.vertex[2].pos);

			translate_triangle(tr, tri);

			result.mult_vec_matrix(tri.vertex[0].normal, tri.vertex[0].normal);
			result.mult_vec_matrix(tri.vertex[1].normal, tri.vertex[1].normal);
			result.mult_vec_matrix(tri.vertex[2].normal, tri.vertex[2].normal);
		}
		result.mult_point_matrix(mesh.bbox_center, mesh.bbox_center);
		translate_triangle(tr, mesh.bbox_center);

	}

	void rot_z(float deg) {

		matrix44 tr(		1,			  0,			0,			0,
							0,			  1,			0,			0,
							0,			  0,			1,			0,
					-mesh.bbox_center.x(), -mesh.bbox_center.y(), -mesh.bbox_center.z(), 1);
		matrix44 itr = tr.inverse();
		float sen = sin(deg*M_PI / 180.0f);
		float co = cos(deg*M_PI / 180.0f);
		matrix44 rot( co, -sen,  0,   0,
					 sen,   co,  0,   0,
					   0,    0,  1,   0,
					   0,    0,  0,   1);
		matrix44 result = (tr*rot)*itr;

		for ( Triangle &tri : mesh.tris) {
			result.mult_point_matrix(tri.vertex[0].pos, tri.vertex[0].pos);
			result.mult_point_matrix(tri.vertex[1].pos, tri.vertex[1].pos);
			result.mult_point_matrix(tri.vertex[2].pos, tri.vertex[2].pos);
			
			result.mult_vec_matrix(tri.vertex[0].normal, tri.vertex[0].normal);
			result.mult_vec_matrix(tri.vertex[1].normal, tri.vertex[1].normal);
			result.mult_vec_matrix(tri.vertex[2].normal, tri.vertex[2].normal);
		}
		result.mult_point_matrix(mesh.bbox_center, mesh.bbox_center);

	}
};