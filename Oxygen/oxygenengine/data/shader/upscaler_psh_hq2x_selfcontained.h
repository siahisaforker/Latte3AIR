#pragma once
const char upscaler_psh_hq2x_selfcontained[] =
R"(  
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D uTexture;

void main()
{
    vec2 texelSize = vec2(1.0)/vec2(textureSize(uTexture,0));
    vec4 c[3][3];
    for(int i=-1;i<=1;i++)
        for(int j=-1;j<=1;j++)
            c[i+1][j+1] = texture(uTexture, TexCoord+vec2(i,j)*texelSize);

    float dx = distance(c[0][1],c[2][1]);
    float dy = distance(c[1][0],c[1][2]);
    vec4 avg = (c[0][1]+c[1][0]+c[1][2]+c[2][1])*0.25;

    FragColor = mix(c[1][1], avg, smoothstep(0.0,0.4,dx+dy));
}
)";
