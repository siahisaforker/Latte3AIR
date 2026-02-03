// xbrz_upscaler_vsh.glsl
attribute vec2 aPos;
attribute vec2 aTex;
varying vec2 vTex;

uniform mat4 uMVP;

void main() {
    vTex = aTex;
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
