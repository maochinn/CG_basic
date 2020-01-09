#pragma once
#include <glm/glm.hpp>

#include "Shader.h"
#include "BufferObject.h"


class DeferredShading
{
public:
    Shader shader_geometry_pass;
    Shader shader_shading_pass;

    FBO fbo;
    VAO vao;
	DeferredShading(Shader shader_geometry_pass, Shader shader_lighting_pass,
        glm::ivec2 resolution):
        shader_geometry_pass(shader_geometry_pass), shader_shading_pass(shader_lighting_pass)
	{
        this->size = resolution;

        this->fbo = this->generateFBO(resolution.x, resolution.y);
        this->vao = this->generateVAO();
	}
    void render()
    {
        this->shader_shading_pass.Use();

        this->bindPositionTexture(0);
        glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gPosition"), 0);
        this->bindNormalTexture(1);
        glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gNormal"), 1);
        this->bindDiffAlbedoTexture(2);
        glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gDiffAlbedo"), 2);
		this->bindSpecAlbedoTexture(3);
		glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gSpecAlbedo"), 3);

        glBindVertexArray(this->vao.buffer);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    void bindFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->fbo.buffer);
    }
    void bindPositionTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, fbo.textures[0]);
    }
    void bindNormalTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, fbo.textures[1]);
    }
    void bindDiffAlbedoTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, fbo.textures[2]);
    }
	void bindSpecAlbedoTexture(int bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, fbo.textures[3]);
	}
private:	
    glm::ivec2 size;

	FBO generateFBO(int width, int height)
	{
        // Set up G-Buffer
        // 4 textures:
        // 1. Positions (RGB)
        // 2. Normals (RGB)
        // 3. diffuse color(RGBA)
        // 4. Specular color(RGBA)
        GLuint gBuffer;
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        GLuint gPosition, gNormal, gDiffAlbedo, gSpecAlbedo;
        // - Position color buffer
        glGenTextures(1, &gPosition);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
        // - Normal color buffer
        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
        // - Diffuse color buffer
        glGenTextures(1, &gDiffAlbedo);
        glBindTexture(GL_TEXTURE_2D, gDiffAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffAlbedo, 0);
        // - Specular color buffer
        glGenTextures(1, &gSpecAlbedo);
        glBindTexture(GL_TEXTURE_2D, gSpecAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSpecAlbedo, 0);

        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);
        // - Create and attach depth buffer (renderbuffer)
        GLuint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // - Finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        FBO fbo;
        fbo.buffer = gBuffer;
        fbo.textures[0] = gPosition;
        fbo.textures[1] = gNormal;
        fbo.textures[2] = gDiffAlbedo;
        fbo.textures[3] = gSpecAlbedo;
        fbo.rbo = rboDepth;
        return fbo;
	}
    VAO generateVAO()
    {
        GLuint quadVAO;
        GLuint quadVBO;

        //generate a quad
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        VAO vao;
        vao.buffer = quadVAO;
        vao.vbo[0] = quadVBO;

        return vao;
    }
};