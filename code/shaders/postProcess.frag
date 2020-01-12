#version 430 core
out vec4 f_color;
  
in V_OUT
{
   vec2 texture_pos;
} f_in;

uniform sampler2D u_hdrColor;
uniform sampler2D u_brightColor;
uniform float u_exposure;


void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_hdrColor, f_in.texture_pos).rgb;

    float kernel[49] = float[](
    0.00000067, 0.00002292,	0.00019117,	0.00038771,	0.00019117,	0.00002292,	0.00000067,
    0.00002292,	0.00078633,	0.00655965,	0.01330373,	0.00655965,	0.00078633,	0.00002292,
    0.00019117,	0.00655965,	0.05472157,	0.11098164,	0.05472157,	0.00655965,	0.00019117,
    0.00038771,	0.01330373,	0.11098164,	0.22508352,	0.11098164,	0.01330373,	0.00038771,
    0.00019117,	0.00655965,	0.05472157,	0.11098164,	0.05472157,	0.00655965,	0.00019117,
    0.00002292,	0.00078633,	0.00655965,	0.01330373,	0.00655965,	0.00078633,	0.00002292,
    0.00000067,	0.00002292,	0.00019117,	0.00038771,	0.00019117,	0.00002292,	0.00000067);

    vec3 bloom = vec3(0.0, 0.0, 0.0);

    for(int j = 0; j < 7; j++)
        for(int i = 0; i < 7; i++) 
    {
        bloom += kernel[i*7+j] * textureOffset(u_brightColor, f_in.texture_pos, ivec2(i-3, j-3)).rgb; 
    }
    hdrColor += bloom;

    // reinhard
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    // exposure
    vec3 result = vec3(1.0) - exp(-hdrColor * u_exposure);
    // also gamma correct while we're at it       
    // result = pow(result, vec3(1.0 / gamma));
    f_color = vec4(result, 1.0f);
    // f_color = vec4(hdrColor, 1.0);
    // f_color = vec4(0.0, 1.0, 0.0, 1.0);
}
