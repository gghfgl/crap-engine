#shader vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 color;

out vec4 v_Color;

void main()
{
	gl_Position = projection * view * model * vec4(a_Position, 1.0f);
	v_Color = color;
}

#shader fragment
#version 450 core
in vec4 v_Color;

out vec4 FragColor;

void main()
{
    FragColor = v_Color;
}
