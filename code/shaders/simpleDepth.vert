#version 430 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texture_pos;

out V_OUT
{
    vec3 world_pos;
    vec2 texture_pos;
} v_out;

//uniform mat4 lightSpaceMatrix;
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
    v_out.texture_pos = texture_pos;
    gl_Position = u_projection * u_view * world_pos;
}