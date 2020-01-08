#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>   

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "BufferObject.h"


struct Material
{
	glm::vec3 Ka;	//ambient
	glm::vec3 Kd;	//diffuse
	glm::vec3 Ks;	//specular
	GLfloat Ns;	//exponent
	GLfloat d;	//alpha
};
struct ModelOBJ
{
	char name[80];
	VAO vao;
	Material material;
	glm::vec3 min_pos;	//minimum position of AABB
	glm::vec3 max_pos;	//maximum position of AABB
};
struct myVertex 
{
	glm::vec3 position;
	glm::vec3 normal;
};

Material getMaterial(const char path[]);
glm::vec3 getNormal(const std::string str);
glm::vec3 getPosition(const std::string str);
void recordFace(const std::string str,
	std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
	std::vector<myVertex>& vertices, std::vector<glm::ivec3>& indices);

void getModelOBJ(ModelOBJ& data, std::vector<glm::ivec3> indices, std::vector<myVertex> vertices);
/*simple load obj file*/
/*include vertex and normal vector*/
std::vector<ModelOBJ> loadOBJ(const char path[]);



void renderOBJ(std::vector<ModelOBJ> obj);
void renderOBJ(ModelOBJ obj);
