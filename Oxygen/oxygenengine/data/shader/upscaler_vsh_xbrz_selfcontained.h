#pragma once
const char upscaler_vsh_xbrz_selfcontained[] =
R"(  
in vec2 aPos;
in vec2 aTexCoord;
out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos.xy,0.0,1.0);
}
)";
