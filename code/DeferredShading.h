#pragma once
#include "Shader.h"
#include "BufferObject.h"


class DeferredShading
{
public:
    Shader shader_geometry_pass;
    Shader shader_lighting_pass;

    FBO fbo;
    VAO vao;
	DeferredShading(Shader shader_geometry_pass, Shader shader_lighting_pass,
        glm::ivec2 resolution):
        shader_geometry_pass(shader_geometry_pass), shader_lighting_pass(shader_lighting_pass)
	{
        this->size = resolution;

        this->fbo = this->generateFBO(resolution.x, resolution.y);
        this->vao = this->generateVAO();
	}
    void render()
    {
        this->shader_lighting_pass.Use();

        this->bindPositionTexture(0);
        glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gPosition"), 0);
        this->bindNormalTexture(1);
        glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gNormal"), 1);
        this->bindAlbedoSpecTexture(2);
        glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gAlbedoSpec"), 2);

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
    void bindAlbedoSpecTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, fbo.textures[2]);
    }
private:	
    glm::ivec2 size;

	FBO generateFBO(int width, int height)
	{
        // Set up G-Buffer
        // 3 textures:
        // 1. Positions (RGB)
        // 2. Normals (RGB)
        // 3. Color (RGB) + Specular (A)
        GLuint gBuffer;
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        GLuint gPosition, gNormal, gAlbedoSpec;
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
        // - Color + Specular color buffer
        glGenTextures(1, &gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
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
        fbo.textures[2] = gAlbedoSpec;
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