#version 330 core

uniform mat4 proj;
uniform mat4 view;
uniform vec3 camPos;
uniform vec3 camUp;
uniform int isPoint;

in vec3 base;
in vec2 texcoord;
in vec3 pos;

out vec4 fColor;
out vec2 fTexcoord;

void main() {
  fColor = vec4(0.0, 1.0, 0.0, 1.0);
  fTexcoord = texcoord;

  vec3 look = normalize(camPos - pos);
  vec3 right = normalize(cross(camUp, look));
  vec3 up = cross(look, right);
  mat4 model = mat4(
    vec4(right, 0),
    vec4(up, 0),
    vec4(look, 0),
    vec4(pos, 1)
  );

  gl_Position = proj * view * model * vec4(base.x, base.y, base.z, 1.0);
}
