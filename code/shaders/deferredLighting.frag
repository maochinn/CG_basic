#version 430 core
layout (location = 0) out vec4 hdr_color;
layout (location = 1) out vec4 bright_color;

out vec4 f_color;
  
in V_OUT
{
   vec2 texture_pos;
} f_in;

uniform sampler2D u_gDiffAlbedo;
uniform sampler2D u_gSpecAlbedo;
uniform sampler2D u_lAmbient;
uniform sampler2D u_lDiffuse;
uniform sampler2D u_lSpecular;


void main()
{             
    vec3 ambient = texture(u_lAmbient, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).a;
    vec3 diffuse = texture(u_lDiffuse, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).rgb * texture(u_gDiffAlbedo, f_in.texture_pos).a;
    vec3 specular = texture(u_lSpecular, f_in.texture_pos).rgb * texture(u_gSpecAlbedo, f_in.texture_pos).rgb * texture(u_gSpecAlbedo, f_in.texture_pos).a;

    f_color = vec4(ambient + diffuse + specular, 1.0);

    hdr_color = f_color;
    float brightness = dot(hdr_color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        bright_color = hdr_color;
    else
        bright_color = vec4(0.0, 0.0, 0.0, 0.0);

    // if (f_color.r > 1.0 || f_color.g > 1.0 || f_color.b > 1.0)
    //     f_color = vec4(1.0, 0.0, 0.0, 1.0);
    // f_color = vec4(0.0, 1.0, 0.0, 1.0);
    // f_color = vec4(texture(u_gDiffAlbedo, f_in.texture_pos).rgb, 1.0f);
    // f_color = vec4(texture(u_lAmbient, f_in.texture_pos).rgb, 1.0f);
    // f_color = vec4(ambient, 1.0);
}