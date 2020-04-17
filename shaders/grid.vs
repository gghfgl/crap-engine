#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 v_TexCoord;

// uniform mat4 u_MVP;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 v_Color;

void main()
{
    // vec4 position = u_MVP * vec4(a_Position, 1.0);
    vec4 position = projection * view * model * vec4(a_Position, 1.0);
    gl_Position = position;

    v_TexCoord = a_TexCoord;
}
