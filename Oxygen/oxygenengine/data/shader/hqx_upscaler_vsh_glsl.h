#pragma once
const char hqx_upscaler_vsh_glsl[] =
"#version 150\n"
"layout(location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"    TexCoord = aTexCoord;\n"
"    gl_Position = vec4(aPos.xy, 0.0, 1.0);\n"
"}\n";
