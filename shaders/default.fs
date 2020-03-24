#version 330 core

in vec4 vColor;

out vec4 FragColor;

// texture samplers
// uniform sampler2D texture1;
// uniform sampler2D texture2;

// uniform vec3 debug_color;

void main()
{
    // linearly interpolate between both textures (80% container, 20% awesomeface)
    // FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    // FragColor = texture(texture1, TexCoord);
   //  FragColor = vec4(debug_color, 1.0);
    FragColor = vColor;
}
