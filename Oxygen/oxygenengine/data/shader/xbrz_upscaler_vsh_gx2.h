#pragma once
// xBRZ vertex shader (self-contained)
const char xbrz_upscaler_vsh_gx2[] =
R"(  
in vec2 aPos;
in vec2 aTexCoord;
out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
)";
