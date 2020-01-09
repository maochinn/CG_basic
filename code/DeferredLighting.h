#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"
#include "BufferObject.h"
#include "Sphere.h"

class DeferredLighting
{
public:
    Shader shader_geometry_pass;
    Shader shader_lighting_pass;
    Shader shader_shading_pass;

    FBO g_buffer;
    FBO l_buffer;
    //render for viewport
    VAO quad;
    //render for point light
    VAO sphere;
    DeferredLighting(Shader shader_geometry_pass, Shader shader_lighting_pass, Shader shader_shading_pass,
        glm::ivec2 resolution) :
        shader_geometry_pass(shader_geometry_pass), 
        shader_lighting_pass(shader_lighting_pass),
        shader_shading_pass(shader_shading_pass)
    {
        this->size = resolution;

        this->g_buffer = this->generateGBuffer(resolution.x, resolution.y);
        this->l_buffer = this->generateLBuffer(resolution.x, resolution.y);
        this->quad = this->generateQuadVAO();
        this->sphere = Sphere::generateVAO();
    }
    void render()
    {
        //this->shader_lighting_pass.Use();

        //this->bindPositionTexture(0);
        //glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gPosition"), 0);
        //this->bindNormalTexture(1);
        //glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gNormal"), 1);
        //this->bindDiffAlbedoTexture(2);
        //glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gDiffAlbedo"), 2);
        //this->bindSpecAlbedoTexture(3);
        //glUniform1i(glGetUniformLocation(this->shader_lighting_pass.Program, "u_gDiffAlbedo"), 3);

        //glBindVertexArray(this->vao.buffer);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //glBindVertexArray(0);
    }
    void lightingPass(int point_amount)
    {
        this->shader_lighting_pass.Use();
        //accummulate for all draw buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        //
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glBindVertexArray(this->sphere.buffer);
        glDrawElementsInstanced(GL_TRIANGLES, this->sphere.elementAmount, GL_UNSIGNED_INT, 0, point_amount);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);

        glDisable(GL_BLEND);
    }
    void render()
    {
        this->shader_shading_pass.Use();

        //this->bindPositionTexture(0);
        //glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gPosition"), 0);
        //this->bindNormalTexture(1);
        //glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gNormal"), 1);
        //this->bindAlbedoSpecTexture(2);
        //glUniform1i(glGetUniformLocation(this->shader_shading_pass.Program, "u_gAlbedoSpec"), 2);

        glBindVertexArray(this->quad.buffer);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    void bindPositionTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, this->g_buffer.textures[0]);
    }
    void bindNormalTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, this->g_buffer.textures[1]);
    }
    void bindDiffAlbedoTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, this->g_buffer.textures[2]);
    }
    void bindSpecAlbedoTexture(int bind_unit)
    {
        glActiveTexture(GL_TEXTURE0 + bind_unit);
        glBindTexture(GL_TEXTURE_2D, this->g_buffer.textures[3]);
    }
private:
    glm::ivec2 size;

    FBO generateGBuffer(int width, int height)
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffAlbedo, 0);
        // - Specular color buffer
        glGenTextures(1, &gSpecAlbedo);
        glBindTexture(GL_TEXTURE_2D, gSpecAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gSpecAlbedo, 0);

        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT2};
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
    FBO generateLBuffer(int width, int height)
    {  
        // Set up L-Buffer
        // 3 textures:
        // 1. Light ambient (RGB)
        // 2. Light diffuse (RGB)
        // 3. Light specular (RGB)
        GLuint gBuffer;
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        GLuint l_ambient, l_diffuse, l_specular;
        // - Position color buffer
        glGenTextures(1, &l_ambient);
        glBindTexture(GL_TEXTURE_2D, l_ambient);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, l_ambient, 0);
        // - Normal color buffer
        glGenTextures(1, &l_diffuse);
        glBindTexture(GL_TEXTURE_2D, l_diffuse);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, l_diffuse, 0);
        // - Diffuse color buffer
        glGenTextures(1, &l_specular);
        glBindTexture(GL_TEXTURE_2D, l_specular);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB16F, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, l_specular, 0);

        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(4, attachments);
        // - Finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        FBO fbo;
        fbo.buffer = gBuffer;
        fbo.textures[0] = l_ambient;
        fbo.textures[1] = l_diffuse;
        fbo.textures[2] = l_specular;
        return fbo;

    }
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
};