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
    float shininess;
};
uniform Material u_material;
uniform bool u_use_normal_map;
uniform bool u_use_displacement_map;

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
    // And the diffuse per-fragment color
    gDiffAlbedo = texture(u_material.texture_diffuse, f_in.texture_pos);
    // Store specular intensity in gAlbedoSpec's alpha component
    gSpecAlbedo = texture(u_material.texture_specular, f_in.texture_pos);
    // gDiffAlbedo = vec4(1.0, 0.0, 0.0, 1.0);
}