#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "BufferObject.h"
class ShadowMap
{
public:
	Shader depth_shader;
	Shader shadow_shader;
	glm::ivec2 resolution;
	ShadowMap(Shader depth_shader, Shader shadow_shader, glm::ivec2 resolution)
		:depth_shader(depth_shader), shadow_shader(shadow_shader)
	{
		this->resolution = resolution;
		this->fbo = this->generateFBO();
	}
	glm::mat4 getViewProjectionMtx() { return this->projection_matrix * this->view_matrix; }
	glm::mat4 getViewMtx() { return this->view_matrix; }
	glm::mat4 getProjectionMtx() { return this->projection_matrix; }
	void setLight(glm::vec3 position, glm::vec3 direction, glm::vec2 range)
	{
		this->projection_matrix = glm::ortho(-range.x, range.x, -range.y, range.y, 0.0f, 1000.0f);
		this->view_matrix = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
	}
	void bindShadowBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo.buffer);
	}
	void bindShadowMap(int bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, this->fbo.textures[0]);
	}
private:
	FBO fbo;
	
	glm::mat4 view_matrix;	//in light space
	glm::mat4 projection_matrix;	//in light space

	FBO generateFBO()
	{
		//FBO
		// Framebuffers
		GLuint depthMapFBO;
		glGenFramebuffers(1, &depthMapFBO);
		// - Create depth texture
		GLuint depthMap;
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->resolution.x, this->resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glEnable(GL_DEPTH_TEST);
		
		FBO fbo;
		fbo.buffer = depthMapFBO;
		fbo.textures[0] = depthMap;
		return fbo;
	}
};