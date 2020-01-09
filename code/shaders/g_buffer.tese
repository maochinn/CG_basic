#version 430 core
layout(triangles, equal_spacing, ccw) in;

// uniform mat4 gVP;
// uniform sampler2D gDisplacementMap;
// uniform float gDispFactor;
// in vec3 WorldPos_ES_in[];
// in vec2 TexCoord_ES_in[];
// in vec3 Normal_ES_in[];

in C_OUT
{
    vec3 world_pos;
    vec3 normal;
    vec2 texture_pos;
} e_in[];

out E_OUT
{
    vec3 world_pos;
    vec3 normal;
    vec2 texture_pos;
} e_out;

// out vec3 WorldPos_FS_in;
// out vec2 TexCoord_FS_in;
// out vec3 Normal_FS_in;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};



vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2);
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2);

void main()
{
    // Interpolate the attributes of the output vertex using the barycentric coordinates
    
    // TexCoord_FS_in = interpolate2D(TexCoord_ES_in[0], TexCoord_ES_in[1], TexCoord_ES_in[2]);
    // Normal_FS_in = interpolate3D(Normal_ES_in[0], Normal_ES_in[1], Normal_ES_in[2]);
    // Normal_FS_in = normalize(Normal_FS_in);
    // WorldPos_FS_in = interpolate3D(WorldPos_ES_in[0], WorldPos_ES_in[1], WorldPos_ES_in[2]);

    e_out.texture_pos = interpolate2D(e_in[0].texture_pos, e_in[1].texture_pos, e_in[2].texture_pos);
    e_out.normal = interpolate3D(e_in[0].normal, e_in[1].normal, e_in[2].normal);
    e_out.normal = normalize(e_out.normal);
    e_out.world_pos = interpolate3D(e_in[0].world_pos, e_in[1].world_pos, e_in[2].world_pos);

    // Displace the vertex along the normal
    // float Displacement = texture(gDisplacementMap, TexCoord_FS_in.xy).x;
    // WorldPos_FS_in += Normal_FS_in * Displacement * gDispFactor;
    // gl_Position = gVP * vec4(WorldPos_FS_in, 1.0);

    gl_Position = u_projection * u_view * vec4(e_out.world_pos, 1.0);
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
} 