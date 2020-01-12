#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_pos;

out V_OUT
{
    flat int light_index;
} v_out;

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
    // float radius;
    vec4 attenuation;
};

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};


const int NR_LIGHTS = 32;
layout (std140, binding = 1) uniform Light
{
    DirectLight u_direct_light;
    PointLight u_point_light[NR_LIGHTS];
};

void main()
{
    float radius = u_point_light[gl_InstanceID].attenuation[3];
    vec4 world_pos = vec4(radius * position + u_point_light[gl_InstanceID].position, 1.0f);

    v_out.light_index = gl_InstanceID;
 
    gl_Position = u_projection * u_view * world_pos;
}