#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_pos;
layout (location = 3) in vec3 diffuse;
layout (location = 4) in vec3 specular;
layout (location = 5) in vec3 ambient;

out V_OUT
{
    vec3 world_pos;
    vec3 normal;
    vec2 texture_pos;
} v_out;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};

uniform mat4 u_model;

void main()
{
    vec4 world_pos = u_model * vec4(position, 1.0f);
    v_out.world_pos = world_pos.xyz; 
    gl_Position = u_projection * u_view * world_pos;
    v_out.texture_pos = texture_pos;
    
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    v_out.normal = normalMatrix * normal;
}