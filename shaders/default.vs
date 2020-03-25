#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

layout (std140) uniform Matrices
{
    mat4 projection;
};

uniform mat4 view;
uniform mat4 model;

out vec4 vColor;

void main()
{
	vColor = aColor;
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	// TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
