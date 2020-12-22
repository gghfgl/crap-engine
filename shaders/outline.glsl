#shader vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 v_TexCoord;

void main()
{
	gl_Position = projection * view * model * vec4(a_Position, 1.0f);

	v_TexCoord = a_TexCoord;
}

#shader fragment
#version 450 core
in vec2 v_TexCoord;

out vec4 FragColor;

void main()
{
	FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}
