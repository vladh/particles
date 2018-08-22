#version 330 core

uniform sampler2D tex;
uniform int isPoint;

in vec4 fColor;
in vec2 fTexcoord;

out vec4 outColor;

void main() {
  if (isPoint == 1) {
    outColor = fColor;
  } else {
    outColor = texture(tex, fTexcoord) * fColor;
  }
}
