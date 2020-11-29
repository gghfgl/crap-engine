#shader vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(a_Position, 1.0f);
}

#shader fragment
#version 450 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
