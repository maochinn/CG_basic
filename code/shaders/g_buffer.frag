#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffAlbedo;
layout (location = 3) out vec4 gSpecAlbedo;

in V_OUT
{
    vec3 world_pos;
    vec3 normal;
    vec2 texture_pos;
} f_in;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};
uniform Material u_material;

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    gPosition = f_in.world_pos;
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(f_in.normal);
    // And the diffuse per-fragment color
    gDiffAlbedo = texture(u_material.texture_diffuse1, f_in.texture_pos);
    // Store specular intensity in gAlbedoSpec's alpha component
    gSpecAlbedo = texture(u_material.texture_specular1, f_in.texture_pos);
    // gDiffAlbedo = vec4(1.0, 0.0, 0.0, 1.0);
}