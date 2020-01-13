#version 430 core
layout (location = 0) out vec4 hdr_color;
layout (location = 1) out vec4 bright_color;

out vec4 f_color;
  
in V_OUT
{
   vec2 texture_pos;
} f_in;

layout (std140, binding = 2) uniform View
{
    vec3 u_view_dir;    //0
    vec3 u_view_pos;    //16
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
    // float radius;
    vec4 attenuation;
};
const int NR_LIGHTS = 32;

layout (std140, binding = 1) uniform Light
{
    DirectLight u_direct_light;
    PointLight u_point_light[NR_LIGHTS];
};

uniform sampler2D u_gDiffAlbedo;
uniform sampler2D u_gSpecAlbedo;
uniform sampler2D u_lAmbient;
uniform sampler2D u_lDiffuse;
uniform sampler2D u_lSpecular;

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_shadowMap;

uniform bool u_useShadowMap;
uniform mat4 u_lightProjViewMtx;

float computeShadow()
{
    vec3 world_pos = texture(u_gPosition, f_in.texture_pos).rgb;
    vec4 light_pos = u_lightProjViewMtx * vec4(world_pos, 1.0);

    // perform perspective divide
    vec3 proj_coords = light_pos.xyz / light_pos.w;
    // Transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closest_depth = texture(u_shadowMap, proj_coords.xy).r; 
    // Get depth of current fragment from light's perspective
    float current_depth = proj_coords.z;
    // Calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(texture(u_gNormal, f_in.texture_pos).rgb);
    vec3 light_dir = normalize(u_view_pos - world_pos);
    float bias = max(0.001 * (1.0 - dot(normal, light_dir)), 0.0001);
    // Check whether current frag pos is in shadow
    // float shadow = current_depth - bias > closest_depth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(u_shadowMap, proj_coords.xy + vec2(x, y) * texel_size).r; 
            shadow += current_depth - bias > pcf_depth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // Keep the shsadow at 0.0 when outside the far_plane region of the light's frustum.
    if(proj_coords.z > 1.0)
        shadow = 0.0;
    
    return shadow;
}

void computeDirectLighting(in DirectLight light,
    in vec3 normal, in vec3 view_dir, 
    inout vec3 l_ambient, inout vec3 l_diffuse, inout vec3 l_specular)
{
   vec3 light_dir = normalize(-light.direction);

	//ambient
	vec3 ambient = light.ambient;	//texture() return vec4

	//diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff;

	//specular
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    vec3 specular = light.specular * spec;

    if(u_useShadowMap)
    {
        // reduce shadow strength a little: allow some diffuse/specular light in shadowed regions       
        float shadow = min(computeShadow(), 0.75);
        l_ambient += ambient;       
        l_diffuse += (1.0 - shadow) * diffuse;
        l_specular += (1.0 - shadow) * specular;
    }
    else
    {
        l_ambient += ambient;
        l_diffuse += diffuse;
        l_specular += specular;
    }
}

void main()
{
    // lighting
    //point lighting
    vec3 ambient = texture(u_lAmbient, f_in.texture_pos).rgb;
    vec3 diffuse = texture(u_lDiffuse, f_in.texture_pos).rgb;
    vec3 specular = texture(u_lSpecular, f_in.texture_pos).rgb;
    // vec3 ambient = vec3(0.0, 0.0, 0.0);
    // vec3 diffuse = vec3(0.0, 0.0, 0.0);
    // vec3 specular = vec3(0.0, 0.0, 0.0);
    //direct lighting
    computeDirectLighting(u_direct_light,
        normalize(texture(u_gNormal, f_in.texture_pos).rgb), u_view_dir,
        ambient, diffuse, specular);

    //material
    ambient *= texture(u_gDiffAlbedo, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).a;
    diffuse *= texture(u_gDiffAlbedo, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).a;
    specular *= texture(u_gSpecAlbedo, f_in.texture_pos).rgb * texture(u_gSpecAlbedo, f_in.texture_pos).a;

    hdr_color = vec4(ambient + diffuse + specular, 1.0);

    float brightness = dot(hdr_color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        bright_color = hdr_color;
    else
        bright_color = vec4(0.0, 0.0, 0.0, 0.0);

    //  f_color = vec4(ambient + diffuse + specular, 1.0);
    f_color = hdr_color;

    // if (f_color.r > 1.0 || f_color.g > 1.0 || f_color.b > 1.0)
    //     f_color = vec4(1.0, 0.0, 0.0, 1.0);
    // f_color = vec4(0.0, 1.0, 0.0, 1.0);
    // f_color = vec4(texture(u_gDiffAlbedo, f_in.texture_pos).rgb, 1.0f);
    // f_color = vec4(texture(u_lAmbient, f_in.texture_pos).rgb, 1.0f);
    // f_color = vec4(ambient, 1.0);
}