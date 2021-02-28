#shader vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;
layout (location = 5) in uvec4 a_boneIds; 
layout (location = 6) in vec4 a_weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 v_TexCoord;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(a_boneIds[i] == -1) 
            continue;
        if(a_boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(a_Position,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[a_boneIds[i]] * vec4(a_Position,1.0f);
        totalPosition += localPosition * a_weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[a_boneIds[i]]) * a_Normal;
    }
	
    gl_Position =  projection * view * model * totalPosition;
    //gl_Position = projection * view * model * vec4(a_Position, 1.0f);
    v_TexCoord = a_TexCoord;
}

#shader fragment
#version 450 core
in vec2 v_TexCoord;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main()
{
    FragColor = texture(texture_diffuse1, v_TexCoord);
}
