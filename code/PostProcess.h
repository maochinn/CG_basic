#pragma once

#include "BufferObject.h"
#include "Shader.h"

class PostProcess
{
public:
    Shader shader;
    FBO hdr_buffer;

    float exposure;
	glm::ivec2 size;
    PostProcess(Shader shader, glm::ivec2 resolution):
        shader(shader)
    {
        this->exposure = 0.1f;
		this->size = resolution;
        this->quad = this->generateQuadVAO();
        this->hdr_buffer = this->generateHDRBuffer(resolution.x, resolution.y);
    }
    void render()
    {
        this->shader.Use();

        this->bindHDRColorTexture(0);
        glUniform1i(glGetUniformLocation(this->shader.Program, "u_hdrColor"), 0);
        this->bindBrightColorTexture(1);
        glUniform1i(glGetUniformLocation(this->shader.Program, "u_brightColor"), 1);
        glUniform1f(glGetUniformLocation(this->shader.Program, "u_exposure"), this->exposure);
       
        glBindVertexArray(this->quad.buffer);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    void bindHDRBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, this->hdr_buffer.buffer); }
    void bindHDRColorTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, this->hdr_buffer.textures[0]);
    }
    void bindBrightColorTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, this->hdr_buffer.textures[1]);
    }
private:
    VAO quad;

    VAO generateQuadVAO()
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
    FBO generateHDRBuffer(int width, int height)
    {
        // Set up hdr-Buffer
        // 1 textures:
        // 1. color (RGBA)
        GLuint hdr_buffer;
        glGenFramebuffers(1, &hdr_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_buffer);
        GLuint hdr_color, bright_color;

        glGenTextures(1, &hdr_color);
        glBindTexture(GL_TEXTURE_2D, hdr_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr_color, 0);

        glGenTextures(1, &bright_color);
        glBindTexture(GL_TEXTURE_2D, bright_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bright_color, 0);

        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLuint attachments[2] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);

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
        fbo.buffer = hdr_buffer;
        fbo.textures[0] = hdr_color;
        fbo.textures[1] = bright_color;
        fbo.rbo = rboDepth;
        return fbo;
    }
};