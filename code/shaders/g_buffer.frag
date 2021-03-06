#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffAlbedo;
layout (location = 3) out vec4 gSpecAlbedo;

in E_OUT
{
    vec3 world_pos;
    vec3 world_normal;
    vec2 texture_pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat3 TBN;
} f_in;

// in V_OUT
// {
//     vec3 world_pos;
//     vec3 world_normal;
//     vec2 texture_pos;
//     vec3 world_tagent;
// } f_in;

struct Material
{
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    sampler2D texture_normal;
    sampler2D texture_displacement;
    samplerCube texture_cubemap;
    float shininess;
};
uniform Material u_material;

uniform bool u_use_diffuse_texture;
uniform bool u_use_specular_texture;
uniform bool u_use_normal_map;



void main()
{    
    vec3 normal = texture(u_material.texture_normal, f_in.texture_pos).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(f_in.TBN * normal);

    // Store the fragment position vector in the first gbuffer texture
    gPosition = f_in.world_pos;
    // Also store the per-fragment normals into the gbuffer
    if(u_use_normal_map)
        gNormal = normalize(normal);
    else
        gNormal = normalize(f_in.world_normal);
    
    if(u_use_diffuse_texture)
        gDiffAlbedo = texture(u_material.texture_diffuse, f_in.texture_pos);
    else
        gDiffAlbedo = vec4(f_in.diffuse, 1.0);

    if(u_use_specular_texture)
        gSpecAlbedo = texture(u_material.texture_specular, f_in.texture_pos);
    else
        gSpecAlbedo = vec4(f_in.specular, 1.0);
}