#pragma once
// xBRZ 2x fragment shader logic
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D uTexture;

void main()
{
    vec2 texelSize = vec2(1.0)/vec2(textureSize(uTexture,0));
    vec4 c[3][3];

    for(int i=-1;i<=1;i++)
        for(int j=-1;j<=1;j++)
            c[i+1][j+1] = texture(uTexture, TexCoord + vec2(i,j)*texelSize);

    // Approximate xBRZ weighting
    vec4 vert = (c[0][1] + c[2][1])*0.5;
    vec4 horz = (c[1][0] + c[1][2])*0.5;

    float diffVert = distance(c[1][1], vert);
    float diffHorz = distance(c[1][1], horz);

    FragColor = mix(c[1][1], diffVert>diffHorz? vert : horz, 0.5);
}
