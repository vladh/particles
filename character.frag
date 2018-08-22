#version 330 core

uniform sampler2D tex;

in vec2 fTexcoord;
in vec4 fColor;

out vec4 outColor;

void main(void) {
  outColor = vec4(1.0, 1.0, 1.0, texture(tex, fTexcoord).r) * fColor;
}
