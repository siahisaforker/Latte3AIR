// xbrz_upscaler_psh.glsl
precision mediump float;

varying vec2 vTex;
uniform sampler2D uTex;

void main() {
    // Sample original texture
    vec4 color = texture2D(uTex, vTex);
    gl_FragColor = color;
}
