#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec4 v_Color;

void main()
{
	gl_Position = projection * view * model * vec4(a_Position, 1.0f);

	v_Color = a_Color;
}
