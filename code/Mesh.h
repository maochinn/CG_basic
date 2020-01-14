#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;
// GL Includes
#include <glad\glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <assimp\types.h>

#include "Texture.h"
#include "BufferObject.h"
#include "Shader.h"

struct Vertex {
	// Position
	glm::vec3 position;
	// Normal
	glm::vec3 normal;
	// TexCoords
	glm::vec2 texture_pos;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 tangent;

};

//struct Texture {
//	GLuint id;
//	string type;
//	aiString path;	// We store the path of the texture to compare with other textures
//};

class Mesh {
public:
	/*  Mesh Data  */
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture2D> textures;

	/*  Functions  */
	// Constructor
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture2D> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// Now that we have all the required data, set the vertex buffers and its attribute pointers.
		this->setupMesh();
	}

	// Render the mesh
	void render(Shader* shader, bool render_patch = false)
	{
		bool use_diffuse_texture = false;
		bool use_specular_texture = false;
		bool use_normal_map = false;
		bool use_displacement_map = false;

		//only bind a texture
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			textures[i].bind(i);
			if (textures[i].type == Texture2D::Type::TEXTURE_DIFFUSE)
			{
				glUniform1i(glGetUniformLocation(shader->Program, "u_material.texture_specular"), i);
				use_diffuse_texture = true;
			}
			else if (textures[i].type == Texture2D::Type::TEXTURE_SPECULAR)
			{
				glUniform1i(glGetUniformLocation(shader->Program, "u_material.texture_specular"), i);
				use_specular_texture = true;
			}
			else if (textures[i].type == Texture2D::Type::TEXTURE_NORMAL)
			{
				glUniform1i(glGetUniformLocation(shader->Program, "u_material.texture_normal"), i);
				use_normal_map = true;
			}
			else if (textures[i].type == Texture2D::Type::TEXTURE_DISPLACEMENT)
			{
				glUniform1i(glGetUniformLocation(shader->Program, "u_material.texture_displacement"), i);
				use_displacement_map = true;
			}
		}
		glUniform1i(glGetUniformLocation(shader->Program, "u_use_diffuse_texture"), use_diffuse_texture);
		glUniform1i(glGetUniformLocation(shader->Program, "u_use_specular_texture"), use_specular_texture);
		glUniform1i(glGetUniformLocation(shader->Program, "u_use_normal_map"), use_normal_map);
		glUniform1i(glGetUniformLocation(shader->Program, "u_use_displacement_map"), use_displacement_map);

		// Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
		glUniform1f(glGetUniformLocation(shader->Program, "u_material.shininess"), 16.0f);

		// Draw mesh
		glBindVertexArray(this->vao.buffer);
		if (render_patch)
		{
			glPatchParameteri(GL_PATCH_VERTICES, 3);
			glDrawElements(GL_PATCHES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
		}
		else
			glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);		
		glBindVertexArray(0);

		// Always good practice to set everything back to defaults once configured.
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			textures[i].unbind(i);
		}
	}

private:
	/*  Render data  */
	//GLuint VAO, VBO, EBO;
	VAO vao;

	/*  Functions    */
	// Initializes all the buffer objects/arrays
	void setupMesh()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->vao.buffer);
		glGenBuffers(1, this->vao.vbo);
		glGenBuffers(1, &this->vao.ebo);

		glBindVertexArray(this->vao.buffer);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->vao.vbo[0]);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vao.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		// Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texture_pos));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, ambient));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, diffuse));

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, specular));

		// Vertex Tangent Coords
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));


		glBindVertexArray(0);
	}
};


