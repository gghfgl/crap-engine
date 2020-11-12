#shader vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

uniform mat4 projection;
uniform mat4 view;

out vec3 v_TexCoord;

void main()
{
    v_TexCoord = a_Position;
    vec4 pos = projection * view * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}  

#shader fragment
#version 450 core
in vec3 v_TexCoord;

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{    
    FragColor = texture(skybox, v_TexCoord);
}
