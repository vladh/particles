#ifndef UTIL_H
#define UTIL_H

#include <GL/glew.h>

/*
Logs an error in a printf style.
*/
void utilLogError(const char* format, ...);

/*
Logs an info message in a printf style.
*/
void utilLogInfo(const char* format, ...);

/*
Logs a GLFW graphics error.
*/
void utilLogGraphicsError(int error, const char* description);

/*
Loads the file at `path` and returns its contents.
*/
char* utilLoadFile(const char* path);

/*
Makes sure that the given shader is loaded.
*/
void utilAssertShaderLoaded(GLuint shader);

/*
Loads the shader source at the given file path.
*/
GLuint utilLoadShader(const char* source, GLenum shaderType);

/*
Makes a shader program given a vertex shader and a fragment shader.
*/
GLuint utilMakeShaderProgram(GLuint vertexShader, GLuint fragmentShader);

/*
Makes a shader program given the paths to a vertex shader and a fragment shader.
*/
GLuint utilMakeShaderProgramWithPaths(const char* vertPath, const char* fragPath);

/*
Returns a random number in the given range.
*/
float utilRandRange(float min, float max);

/*
Constrains `x` to be between `min` and `max`.
*/
float utilClamp(float x, float min, float max);

#endif
