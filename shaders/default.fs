#version 450 core

//in vec4 v_Color;
in vec2 v_TexCoord;
// in float vTexIndex;

// texture samplers
// uniform sampler2D uTextures[1];
uniform sampler2D texture_diffuse1;
// uniform sampler2D texture2;
// uniform sampler2D uTextures[2];

// uniform vec3 debug_color;

out vec4 FragColor;

void main()
{
    // linearly interpolate between both textures (80% container, 20% awesomeface)
    // FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    // FragColor = texture(texture1, TexCoord);
   //  FragColor = vec4(debug_color, 1.0);
    //FragColor = v_Color;
    //int index = int(vTexIndex);
    FragColor = texture(texture_diffuse1, v_TexCoord);
}
