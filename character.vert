#version 330 core

uniform mat4 proj;

in vec2 pos;
in vec2 texcoord;
in vec3 color;

out vec4 fColor;
out vec2 fTexcoord;

void main(void) {
  fColor = vec4(color, 1.0);
  fTexcoord = texcoord;

  gl_Position = proj * vec4(pos, 0.0, 1.0);
}
