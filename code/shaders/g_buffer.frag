#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffAlbedo;
layout (location = 3) out vec4 gSpecAlbedo;

in E_OUT
{
    vec3 world_pos;
    vec2 texture_pos;
    vec3 world_normal;
    mat3 TBN;
} f_in;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    sampler2D displacement;
    float shininess;
};
uniform Material u_material;

void main()
{    
    vec3 normal = texture(u_material.normal, f_in.texture_pos).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(f_in.TBN * normal);

    // Store the fragment position vector in the first gbuffer texture
    gPosition = f_in.world_pos;
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(f_in.world_normal);
    // And the diffuse per-fragment color
    gDiffAlbedo = texture(u_material.diffuse, f_in.texture_pos);
    // Store specular intensity in gAlbedoSpec's alpha component
    gSpecAlbedo = texture(u_material.specular, f_in.texture_pos);
    // gDiffAlbedo = vec4(1.0, 0.0, 0.0, 1.0);
}