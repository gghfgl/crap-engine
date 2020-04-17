#version 450 core
layout (location = 0) out vec4 FragColor;

uniform float u_Scale;
uniform float u_Res;

in vec2 v_TexCoord;

float grid(vec2 st, float res)
{
    vec2 grid = fract(st);
    return step(res, grid.x) * step(res, grid.y);
}
 
void main()
{
    float scale = u_Scale;
    float resolution = u_Res;

    float x = grid(v_TexCoord * scale, resolution);
    FragColor = vec4(vec3(0.2), 0.5) * (1.0 - x);
}
