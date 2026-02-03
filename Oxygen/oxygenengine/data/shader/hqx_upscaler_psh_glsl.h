#pragma once
const char hqx_upscaler_psh_glsl[] =
"#version 150\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    FragColor = texture(uTexture, TexCoord);\n"
"}\n";
