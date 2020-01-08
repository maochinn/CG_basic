#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in V_OUT
{
   vec3 normal;
   vec3 world_pos;
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
    gAlbedoSpec.rgb = texture(u_material.texture_diffuse1, f_in.texture_pos).rgb;
    // Store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(u_material.texture_specular1, f_in.texture_pos).r;
    // gAlbedoSpec = vec4(1.0, 0.0, 0.0, 1.0);
}