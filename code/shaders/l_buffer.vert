#version 430 core
layout (location = 0) in vec3 position;

// out V_OUT
// {
//    vec3 normal;
//    vec3 world_pos;
//    vec2 texture_pos;
// } v_out;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};
struct DirectLight
{                   //offset
	vec3 direction; //0
    vec3 ambient;   //16
    vec3 diffuse;   //32
    vec3 specular;  //48
};
struct PointLight
{
	vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	// float constant;
	// float linear;
	// float quadratic;
    vec3 attenuation;
};

const int NR_LIGHTS = 32;
layout (std140, binding = 1) uniform Light
{
    DirectLight u_direct_light;
    PointLight u_point_light[NR_LIGHTS];
};
uniform mat4 u_model;

void main()
{
    vec4 world_pos = u_model * vec4(position, 1.0f);
    world_pos += u_point_light[gl_InstanceID].position
    gl_Position = u_projection * u_view * world_pos;
}