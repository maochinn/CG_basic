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


void main()
{   
    //for point light

    //window pos = [0, 1]
    vec2 window_pos = (gl_FragCoord.xy / u_viewport_size.xy);

    vec3 world_pos = texture(u_gPosition, window_pos).rgb;
    vec3 normal = texture(u_gNormal, window_pos).rgb;
    vec3 view_dir = normalize(u_view_pos - world_pos);
    
    vec3 light_pos = u_point_light[f_in.light_index].position;
    vec3 light_dir = normalize(light_pos - world_pos);


	//ambient
	vec3 ambient = u_point_light[f_in.light_index].ambient;

	//diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = u_point_light[f_in.light_index].diffuse * diff;

	//specular
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    vec3 specular = u_point_light[f_in.light_index].specular * spec;


    float distance = length(light_pos - world_pos);
    float attenuation = 1.0f / 
        (u_point_light[f_in.light_index].attenuation[0] +
         u_point_light[f_in.light_index].attenuation[1] * distance +
         u_point_light[f_in.light_index].attenuation[2] * (distance*distance));
    
    lAmbient = vec4(ambient * attenuation, 1.0);
    // lAmbient = vec4(0.0, 0.3, 0.0, 1.0);
    // lAmbient = vec4(normal, 1.0);
    lDiffuse = vec4(diffuse * attenuation, 1.0);
    lSpecular = vec4(specular * attenuation, 1.0);
}