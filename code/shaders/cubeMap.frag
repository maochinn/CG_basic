#version 330 core
out vec4 f_color;

in vec3 texture_pos;

struct Material
{
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    sampler2D texture_normal;
    sampler2D texture_displacement;
    samplerCube texture_cubemap;
    float shininess;
};
uniform Material u_material;

void main()
{    
    f_color = texture(u_material.texture_cubemap, texture_pos);
    //f_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}