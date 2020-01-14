#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <glad\glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Texture.h"
#include "Mesh.h"
//
//GLint TextureFromFile(const char* path, string directory);
//GLint loadPNG_bind_texture(const char* imagepath);

class Model
{
public:
	Shader shader;

	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	Model(const GLchar* path, Shader shader):
		shader(shader)
	{
		this->loadModel(path);
	}

	// Draws the model, and thus all its meshes
	//void Draw(Shader shader)
	//{
	//	for (GLuint i = 0; i < this->meshes.size(); i++)
	//		this->meshes[i].Draw(shader);
	//}
	void render(glm::mat4 model_matrix, Shader* shader = nullptr)
	{
		if (shader == nullptr)
			shader = &(this->shader);

		shader->Use();
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		for (Mesh& mesh : this->meshes)
		{
			if ((shader->type & Shader::Type::TESS_EVALUATION_SHADER) == 0)
				mesh.render(shader, false);
			else
				mesh.render(shader, true);
		}	
	}

private:
	/*  Model Data  */
	vector<Mesh> meshes;
	string directory;

	/*  Functions   */
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string path)
	{
		// Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/'));

		// Process ASSIMP's root node recursively
		this->processNode(scene->mRootNode, scene);
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene. 
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}
		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}

	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// Data to fill
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture2D> textures;

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
			// Positions
			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			// Normals
			if (mesh->HasNormals())
				vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			// Texture Coordinates
			if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
				vertex.texture_pos = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			else
				vertex.texture_pos = glm::vec2(0.0f, 0.0f);
			// Tangent
			if (mesh->HasTangentsAndBitangents())
				vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);

			aiColor3D ambi(0.f, 0.f, 0.f);
			aiColor3D diff(0.f, 0.f, 0.f);
			aiColor3D spec(0.f, 0.f, 0.f);
			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0)	//have no texture
			{
				mat->Get(AI_MATKEY_COLOR_AMBIENT, ambi);
				mat->Get(AI_MATKEY_COLOR_DIFFUSE, diff);
				mat->Get(AI_MATKEY_COLOR_SPECULAR, spec);
			}
			vertex.ambient = glm::vec3(ambi.r, ambi.g, ambi.b);
			vertex.diffuse = glm::vec3(diff.r, diff.g, diff.b);
			vertex.specular = glm::vec3(spec.r, spec.g, spec.b);

			vertices.push_back(vertex);
		}
		// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// Retrieve all indices of the face and store them in the indices vector
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// Process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// 1. Diffuse maps
			vector<Texture2D> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. Specular maps
			vector<Texture2D> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR);
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			// 3. normal maps
			std::vector<Texture2D> normalMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT);
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. displacement maps
			std::vector<Texture2D> displacementMaps = this->loadMaterialTextures(material, aiTextureType_DISPLACEMENT);
			textures.insert(textures.end(), displacementMaps.begin(), displacementMaps.end());
		}

		// Return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture2D> loadMaterialTextures(aiMaterial* mat, aiTextureType type)
	{
		vector<Texture2D> textures;

		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString file_name;

			mat->GetTexture(type, i, &file_name);
			string file_path = this->directory + '/' + string(file_name.C_Str());

			if (type == aiTextureType_DIFFUSE)
				textures.push_back(Texture2D(file_path.c_str(), Texture2D::Type::TEXTURE_DIFFUSE));
			else if (type == aiTextureType_SPECULAR)
				textures.push_back(Texture2D(file_path.c_str(), Texture2D::Type::TEXTURE_SPECULAR));
			else if (type == aiTextureType_HEIGHT)
				textures.push_back(Texture2D(file_path.c_str(), Texture2D::Type::TEXTURE_NORMAL));
			else if (type == aiTextureType_DISPLACEMENT)
				textures.push_back(Texture2D(file_path.c_str(), Texture2D::Type::TEXTURE_DISPLACEMENT));
		}
		return textures;
	}
};

//
//
//GLint TextureFromFile(const char* path, string directory)
//{
//	//Generate texture ID and load texture data 
//	string filename = string(path);
//	filename = directory + '/' + filename;
//	GLint textureID = load_texture(filename.c_str());
//	return textureID;
//}
//
//GLint load_texture(const char* imagepath)
//{
//
//	IplImage* image = cvLoadImage(imagepath, CV_LOAD_IMAGE_UNCHANGED);
//
//	if (!image)
//	{
//		std::cout << imagepath << endl;
//		puts("load image error!");
//		return 0;
//	}
//	for (int i = 0; i < image->width; i++)
//	{
//		for (int j = 0; j < image->height; j++)
//		{
//			CvScalar s = cvGet2D(image, j, i);
//			CvScalar a = cvScalar(s.val[2], s.val[1], s.val[0], s.val[3]);
//			cvSet2D(image, j, i, a);
//		}
//	}
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_2D, textureID);
//	if (image->nChannels == 4)
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData);
//	else
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->imageData);
//	glGenerateMipmap(GL_TEXTURE_2D);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	cvReleaseImage(&image);
//	return textureID;
//}