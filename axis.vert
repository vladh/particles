#version 330 core

uniform mat4 proj;
uniform mat4 view;
uniform vec3 camPos;
uniform vec3 up;

in vec3 pos;
in vec3 color;

out vec3 fColor;

void main() {
  fColor = color;
  gl_Position = proj * view * vec4(pos, 1.0);
}
