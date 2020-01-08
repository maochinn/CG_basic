#version 430 core
out vec4 f_color;
  
in V_OUT
{
   vec2 texture_pos;
} f_in;


uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedoSpec;

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
layout (std140, binding = 2) uniform View
{
    vec3 u_view_dir;    //0
    vec3 u_view_pos;    //16
};


vec3 computeDirectLight(DirectLight light, vec3 normal, vec3 view_dir, vec3 albedo, float specular);
vec3 computePointLight(PointLight light, vec3 normal, vec3 world_pos, vec3 view_dir, vec3 albedo, float specular);

void main()
{             
    // retrieve data from G-buffer
    vec3 world_pos = texture(u_gPosition, f_in.texture_pos).rgb;
    vec3 normal = texture(u_gNormal, f_in.texture_pos).rgb;
    vec3 albedo = texture(u_gAlbedoSpec, f_in.texture_pos).rgb;
    float specular = texture(u_gAlbedoSpec, f_in.texture_pos).a;
    
    // then calculate lighting as usual
    vec3 lighting = albedo * 0.1; // hard-coded ambient component
    vec3 view_dir = normalize(u_view_pos - world_pos);

    lighting += computeDirectLight(u_direct_light, normal, u_view_dir, albedo, specular);

    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        // vec3 light_dir = normalize(lights[i].Position - world_pos);
        // vec3 diffuse = max(dot(Normal, light_dir), 0.0) * Albedo * lights[i].Color;
        // lighting += diffuse;
        lighting += computePointLight(u_point_light[i], normal, world_pos, view_dir, albedo, specular);
    }
    
    f_color = vec4(lighting, 1.0);

    // f_color = vec4(albedo.rgb, 1.0);
    // f_color = vec4(1.0, 0.0, 0.0, 1.0);
    // f_color = vec4(u_point_light[0].attenuation, 1.0f);
    // f_color = vec4(u_point_light[1].ambient, 1.0f);
}

vec3 computeDirectLight(DirectLight light, vec3 normal, vec3 view_dir, vec3 albedo, float specular)
{
   vec3 light_dir = normalize(-light.direction);

	//ambient
	vec3 ambient = light.ambient * albedo;	//texture() return vec4

	//diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * albedo;

	//specular
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    vec3 specular_color = light.specular * spec * specular;

	return (ambient + diffuse + specular_color);
}
vec3 computePointLight(PointLight light, vec3 normal, vec3 world_pos, vec3 view_dir, vec3 albedo, float specular)
{
	vec3 light_dir = normalize(light.position - world_pos);

	//ambient
	vec3 ambient = light.ambient * albedo;	//texture() return vec4

	//diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * albedo;

	//specular
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    vec3 specular_color = light.specular * spec * specular;

	//attenuation
	float distance = length(light.position - world_pos);
	// float attenuation = 1.0f / (light.constant + light.linear*distance +light.quadratic*(distance*distance));
    float attenuation = 1.0f / 
        (light.attenuation[0] + light.attenuation[1]*distance +light.attenuation[2]*(distance*distance));	

	ambient *= attenuation;
	diffuse *= attenuation;
	specular_color *= attenuation;

	return (ambient + diffuse + specular_color);
}