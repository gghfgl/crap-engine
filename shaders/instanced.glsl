#shader vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;
layout (location = 5) in mat4 a_InstanceMatrix;

uniform mat4 projection;
uniform mat4 view;
uniform uint flip_color;

out vec2 v_TexCoord;
flat out uint v_FlipColor;

void main()
{
	gl_Position = projection * view * a_InstanceMatrix * vec4(a_Position, 1.0f);

	v_TexCoord = a_TexCoord;
	v_FlipColor = flip_color;
}

#shader fragment
#version 450 core
in vec2 v_TexCoord;
flat in uint v_FlipColor;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main()
{
    if (v_FlipColor != 0)
	FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
	FragColor = texture(texture_diffuse1, v_TexCoord);
}
