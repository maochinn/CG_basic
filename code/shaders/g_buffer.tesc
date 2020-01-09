#version 430 core
// define the number of CPs in the output patch
layout (vertices = 3) out;
// attributes of the input CPs
in V_OUT
{
    vec3 world_pos;
    vec3 normal;
    vec2 texture_pos;
} c_in[];

out C_OUT
{
    vec3 world_pos;
    vec3 normal;
    vec2 texture_pos;
} c_out[];


layout (std140, binding = 2) uniform View
{
    vec3 u_view_dir;    //0
    vec3 u_view_pos;    //16
};


// in vec3 WorldPos_CS_in[];
// in vec2 TexCoord_CS_in[];
// in vec3 Normal_CS_in[];
// // attributes of the output CPs
// out vec3 WorldPos_ES_in[];
// out vec2 TexCoord_ES_in[];
// out vec3 Normal_ES_in[];

float getTessLevel(float distance_0, float distance_1);

void main()
{
    // Set the control points of the output patch
    // TexCoord_ES_in[gl_InvocationID] = TexCoord_CS_in[gl_InvocationID];
    // Normal_ES_in[gl_InvocationID] = Normal_CS_in[gl_InvocationID];
    // WorldPos_ES_in[gl_InvocationID] = WorldPos_CS_in[gl_InvocationID];
    c_out[gl_InvocationID].world_pos = c_in[gl_InvocationID].world_pos;
    c_out[gl_InvocationID].normal = c_in[gl_InvocationID].normal;
    c_out[gl_InvocationID].texture_pos = c_in[gl_InvocationID].texture_pos;

    // Calculate the distance from the camera to the three control points
    // float EyeToVertexDistance0 = distance(gEyeWorldPos, WorldPos_ES_in[0]);
    // float EyeToVertexDistance1 = distance(gEyeWorldPos, WorldPos_ES_in[1]);
    // float EyeToVertexDistance2 = distance(gEyeWorldPos, WorldPos_ES_in[2]);
    float distance_0 = distance(u_view_pos, c_out[0].world_pos);
    float distance_1 = distance(u_view_pos, c_out[1].world_pos);
    float distance_2 = distance(u_view_pos, c_out[2].world_pos);

    // Calculate the tessellation levels
    gl_TessLevelOuter[0] = getTessLevel(distance_1, distance_2);
    gl_TessLevelOuter[1] = getTessLevel(distance_2, distance_0);
    gl_TessLevelOuter[2] = getTessLevel(distance_0, distance_1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];

    // gl_TessLevelOuter[0] = 2.0;
    // gl_TessLevelOuter[1] = 2.0;
    // gl_TessLevelOuter[2] = 2.0;
    // gl_TessLevelInner[0] = 3.0;
}

float getTessLevel(float distance_0, float distance_1)
{
    float AvgDistance = (distance_0 + distance_1) / 2.0;
    if (AvgDistance <= 2.0) {
        return 10.0;
    }
    else if (AvgDistance <= 5.0) {
        return 7.0;
    }
    else {
        return 3.0;
    }
} 