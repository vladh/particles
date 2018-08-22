#include <GL/glew.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void utilLogError(const char* format, ...) {
  va_list vargs;
  printf("[error] ");
  va_start(vargs, format);
  vprintf(format, vargs);
  printf("\n");
  va_end(vargs);
}

void utilLogInfo(const char* format, ...) {
  va_list vargs;
  printf("[info] ");
  va_start(vargs, format);
  vprintf(format, vargs);
  printf("\n");
  va_end(vargs);
}

void utilLogGraphicsError(int error, const char* description) {
  utilLogError("[printGraphicsError] %d %s", error, description);
}

char* utilLoadFile(const char* path) {
  FILE* f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* string = (char*)malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = 0;

  return string;
}

void utilAssertShaderStatusOk(GLuint shader) {
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  char message[512];
  glGetShaderInfoLog(shader, 512, NULL, message);

  utilLogInfo("[assertShaderStatusOk] Compilation for shader %d", shader);
  utilLogInfo("Status: %d", status);
  utilLogInfo("Message: %s", message);

  if (status == GL_FALSE) {
    utilLogError("[assertShaderStatusOk] Shader compilation failed");
    exit(EXIT_FAILURE);
  }

  printf("\n");
}

void utilAssertProgramStatusOk(GLuint shader) {
  GLint status;
  glGetProgramiv(shader, GL_COMPILE_STATUS, &status);

  char message[512];
  glGetProgramInfoLog(shader, 512, NULL, message);

  utilLogInfo("[assertProgramStatusOk] Loading program");
  utilLogInfo("Status: %d", status);
  utilLogInfo("Message: %s", message);

  if (status == GL_FALSE) {
    utilLogError("[assertProgramStatusOk] Program loading failed");
    exit(EXIT_FAILURE);
  }

  printf("\n");
}

GLuint utilLoadShader(const char* source, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  utilAssertShaderStatusOk(shader);
  return shader;
}

GLuint utilMakeShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  utilAssertProgramStatusOk(shaderProgram);
  return shaderProgram;
}

GLuint utilMakeShaderProgramWithPaths(const char* vertPath, const char* fragPath) {
  return utilMakeShaderProgram(
    utilLoadShader(utilLoadFile(vertPath), GL_VERTEX_SHADER),
    utilLoadShader(utilLoadFile(fragPath), GL_FRAGMENT_SHADER)
  );
}

float utilRandRange(float min, float max) {
  float random = ((float) rand()) / (float) RAND_MAX;
  return min + (random * (max - min));
}

float utilClamp(float x, float min, float max) {
  if (x < min) { return min; }
  if (x > max) { return max; }
  return x;
}
