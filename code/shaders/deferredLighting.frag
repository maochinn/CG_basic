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
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
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

void main()
{             
    vec3 ambient = texture(u_lAmbient, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).a;
    vec3 diffuse = texture(u_lDiffuse, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).a;
    vec3 specular = texture(u_lSpecular, f_in.texture_pos).rgb * texture(u_gSpecAlbedo, f_in.texture_pos).rgb * texture(u_gSpecAlbedo, f_in.texture_pos).a;

    if(u_useShadowMap)
    {
        float shadow = computeShadow();                      
        shadow = min(shadow, 0.75); // reduce shadow strength a little: allow some diffuse/specular light in shadowed regions
        hdr_color = vec4((ambient + (1.0 - shadow) * (diffuse + specular)), 1.0);
        // hdr_color = vec4(texture(u_gDiffAlbedo, f_in.texture_pos).rgb, 1.0f);
        // hdr_color = vec4(
        //     texture(u_shadowMap, f_in.texture_pos).r,
        //     texture(u_shadowMap, f_in.texture_pos).r,
        //     texture(u_shadowMap, f_in.texture_pos).r,
        //     1.0);
    }
    else
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