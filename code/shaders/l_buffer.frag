#version 430 core
layout(early_fragment_tests) in;

layout (location = 0) out vec3 l_ambient;
layout (location = 1) out vec3 l_diffuse;
layout (location = 2) out vec4 l_specular;

// in V_OUT
// {
//    vec3 normal;
//    vec3 world_pos;
//    vec2 texture_pos;
// } f_in;

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

void main()
{    
    l_ambient = u_point_light[gl_InstanceID].ambient * u_point_light[gl_InstanceID].attenuation;
    l_diffuse = u_point_light[gl_InstanceID].diffuse * u_point_light[gl_InstanceID].attenuation;
    l_specular = u_point_light[gl_InstanceID].specular * u_point_light[gl_InstanceID].attenuation;
}