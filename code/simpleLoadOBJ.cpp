#include "simpleLoadOBJ.h"


Material getMaterial(const char path[], Material& material)
{
	std::stringstream stream("empty string!");
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		file.open(path);
		stream << file.rdbuf();
		file.close();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::MTLLIB::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	

	while (!stream.eof())
	{
		std::string str;
		stream >> str;
		if (str[0] == '#')
		{
			//comment
		}
		else if (str[0] == 'K' && str[1] == 'a')
		{
			stream >> material.Ka.r;
			stream >> material.Ka.g;
			stream >> material.Ka.b;
		}
	}
}
glm::vec3 getNormal(const std::string str)
{
	//str[0:2] = "vn "
	auto s = str.substr(3, str.size() - 3);
	std::string::size_type sz;
	std::string::size_type idx = 0;
	float x = std::stof(s, &sz);
	idx += sz + 1;
	float y = std::stof(s.substr(idx), &sz);
	idx += sz + 1;
	float z = std::stof(s.substr(idx), &sz);

	return glm::vec3{ x, y ,z };

}
glm::vec3 getPosition(const std::string str)
{
	//str[0:2] = "v "
	auto s = str.substr(2, str.size() - 2);
	std::string::size_type sz;
	std::string::size_type idx = 0;
	float x = std::stof(s, &sz);
	idx += sz + 1;
	float y = std::stof(s.substr(idx), &sz);
	idx += sz + 1;
	float z = std::stof(s.substr(idx), &sz);

	return glm::vec3{ x, y ,z };
}
void recordFace(const std::string str,
	std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
	std::vector<myVertex>& vertices, std::vector<glm::ivec3>& indices)
{
	//str[0:2] = "f "
	auto s = str.substr(2, str.size() - 2);
	std::string::size_type sz;
	std::string::size_type idx = 0;

	int vertexNum = 0;
	while (idx < s.size())
	{
		int pos, uv, nor;
		pos = std::stoi(s.substr(idx), &sz);
		idx += sz + 1;
		if (s[idx] == '/')
		{
			//have no uv index;
			idx++;
		}
		else
		{
			uv = std::stoi(s.substr(idx), &sz);
			idx += sz + 1;
		}
		nor = std::stoi(s.substr(idx), &sz);
		idx += sz + 1;

		myVertex temp = { positions[pos - 1], normals[nor - 1] };
		vertices.push_back(temp);

		vertexNum++;
	}
	//let polygon to triangle
	for (int i = vertices.size() - vertexNum; i + 2 < vertices.size(); i++)
	{
		int x = vertices.size() - vertexNum;
		int y = i + 1;
		int z = i + 2;
		glm::ivec3 temp = { x, y, z };
		indices.push_back(temp);
	}
}
void getModelOBJ(ModelOBJ& data, std::vector<glm::ivec3> indices, std::vector<myVertex> vertices)
{
	VAO vao;

	glGenVertexArrays(1, &vao.vao);
	glGenBuffers(1, vao.vbo);
	glGenBuffers(1, &vao.ebo);

	glBindVertexArray(vao.vao);

	glBindBuffer(GL_ARRAY_BUFFER, vao.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(myVertex), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(myVertex), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(myVertex), (GLvoid*)offsetof(myVertex, normal));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::ivec3), &indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);

	vao.elementAmount = indices.size() * 3;

	//find min and max position
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	for (auto v : vertices)
	{
		if (v.position.x < min.x)
			min.x = v.position.x;
		if (v.position.y < min.y)
			min.y = v.position.y;
		if (v.position.z < min.z)
			min.z = v.position.z;

		if (v.position.x > max.x)
			max.x = v.position.x;
		if (v.position.y > max.y)
			max.y = v.position.y;
		if (v.position.z > max.z)
			max.z = v.position.z;
	}
	data.vao = vao;
	data.min_pos = min;
	data.max_pos = max;
	return;
}
/*simple load obj file*/
/*include vertex and normal vector*/
std::vector<ModelOBJ> loadOBJ(const char path[])
{
	std::string OBJ_str("empty string!");
	std::ifstream OBJ_File;
	OBJ_File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		std::stringstream OBJ_Stream;
		OBJ_File.open(path);
		OBJ_Stream << OBJ_File.rdbuf();
		OBJ_File.close();
		OBJ_str = OBJ_Stream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	//create vector to record vertex info
	//to create VAO
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::ivec3> indices;
	std::vector<myVertex> vertices;
	std::vector<ModelOBJ> obj_models;
	std::string input;

	size_t nowIdx = 0;
	size_t endIdx = OBJ_str.find('\n');
	while (nowIdx < OBJ_str.size())
	{
		auto str = OBJ_str.substr(nowIdx, (endIdx - nowIdx));
		if (str[0] == '#')
		{
			//is comment
		}
		else if (str[0] == 'm' && str[1] == 't' && str[2] == 'l')
		{
			//mtllib
			std::stringstream path(str);
			std::string name;
			path >> name;
			path >> name;

			
		}
		else if (str[0] == 'o')
		{
			//is object
			if (vertices.empty() == false)
				getModelOBJ(obj_models.back(), indices, vertices);

			auto name = str.substr(2, (str.find('\n') - 2));
			ModelOBJ obj;
			//set default material
			{
				obj.material.Ka = glm::vec3(0.2f);
				obj.material.Kd = glm::vec3(0.3f);
				obj.material.Ks = glm::vec3(0.1f);
				obj.material.d = 1.0f;
				obj.material.Ns = 16.0f;
			}

			//std::size_t length = name.copy(obj.name, name.size(), 0);
			std::size_t length = std::min(strlen(name.c_str()), (std::size_t)80);
			strncpy_s(obj.name, name.c_str(), length);
			obj_models.push_back(obj);

			//positions.clear();
			//normals.clear();
			vertices.clear();
			indices.clear();
		}
		else if (str[0] == 'v' && str[1] == 'n')
		{
			//std::cout << str << std::endl;
			normals.push_back(getNormal(str));
		}
		else if (str[0] == 'v')
		{
			//std::cout << str << std::endl;
			positions.push_back(getPosition(str));
		}
		else if (str[0] == 'f')
		{
			//std::cout << str << std::endl;
			if (positions.size() > normals.empty())
			{
				for (int i = normals.size(); i<positions.size(); i++)
					normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
			}
			recordFace(str, positions, normals, vertices, indices);
		}
		else
		{
			//
		}
		nowIdx += (str.size() + 1);
		endIdx = OBJ_str.find('\n', nowIdx);

		if (endIdx == std::string::npos)
			break;
	}
	//last obj data
	if (vertices.empty() == false)
		getModelOBJ(obj_models.back(), indices, vertices);
	return obj_models;

}
void renderOBJ(std::vector<ModelOBJ> obj)
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	for (auto o : obj)
	{
		glBindVertexArray(o.vao.vao);
		glDrawElements(GL_TRIANGLES, o.vao.elementAmount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
void renderOBJ(ModelOBJ obj)
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	glBindVertexArray(obj.vao.vao);
	glDrawElements(GL_TRIANGLES, obj.vao.elementAmount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
