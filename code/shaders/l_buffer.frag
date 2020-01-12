#version 430 core
layout(early_fragment_tests) in;

layout (location = 0) out vec4 lAmbient;
layout (location = 1) out vec4 lDiffuse;
layout (location = 2) out vec4 lSpecular;

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

in V_OUT
{
    flat int light_index;
} f_in;



const int NR_LIGHTS = 32;
layout (std140, binding = 1) uniform Light
{
    DirectLight u_direct_light;
    PointLight u_point_light[NR_LIGHTS];
};

layout (std140, binding = 2) uniform View
{
    vec3 u_view_dir;    //0
    vec3 u_view_pos;    //16
};

uniform ivec2 u_viewport_size;

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;



void computePointLighting(in PointLight light,
    in vec3 normal, vec3 world_pos, 
    inout vec3 l_ambient, inout vec3 l_diffuse, inout vec3 l_specular)
{
    vec3 light_pos = light.position;

    vec3 light_dir = normalize(light_pos - world_pos);
    vec3 view_dir = normalize(u_view_pos - world_pos);

	//ambient
	vec3 ambient = light.ambient;

	//diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff;

	//specular
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    vec3 specular = light.specular * spec;


    float distance = length(light_pos - world_pos);
    float attenuation = 1.0f / 
        (light.attenuation[0] +
         light.attenuation[1] * distance +
         light.attenuation[2] * (distance*distance));

    l_ambient += ambient * attenuation;
    l_diffuse += diffuse * attenuation;
    l_specular += specular * attenuation;
}


void main()
{   
    //for point light
    //window pos = [0, 1]
    vec2 window_pos = (gl_FragCoord.xy / u_viewport_size.xy);

    vec3 ambient = vec3(0.0, 0.0, 0.0);
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);

    computePointLighting(u_point_light[f_in.light_index], 
        texture(u_gNormal, window_pos).rgb, texture(u_gPosition, window_pos).rgb, 
        ambient, diffuse, specular);

    
    lAmbient = vec4(ambient, 1.0);
    lDiffuse = vec4(diffuse, 1.0);
    lSpecular = vec4(specular, 1.0);
}