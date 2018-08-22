/*
References:
[buffer-orphaning] https://www.opengl.org/wiki/Buffer_Object_Streaming
[logl-text-rendering] http://learnopengl.com/#!In-Practice/Text-Rendering
*/

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <time.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "util.hpp"
#include "emitter.hpp"
#include "attractor.hpp"
#include "physics.hpp"
#include "texutil.hpp"
#include "controls.hpp"
#include "character.hpp"
#include "sound.hpp"

const bool isAlpacaModeOn = false;

const float RESOLUTION_SCALE = 2.0f;
GLfloat _windowWidth, _windowHeight, _aspectRatio;
char _windowTitle[128];
GLFWwindow* _window;

GLfloat _tDeltaLastFrame = 0.0f;
GLfloat _tLastFrame = 0.0f;

unsigned _fpsNrFrames;
float _fpsLastTime = 0.0f, _fpsLast = 0.0f;
unsigned long _nrFrame = 0;

typedef enum {POINT, QUAD} DrawingMode;
DrawingMode _drawingMode = QUAD;

const GLfloat CAM_BASE_SPEED = 50.0f;
GLfloat _camSpeed, _zNear = 1.0f, _zFar = 2000.0f;
glm::mat4 _view, _proj, _orthoProj;
glm::vec3 _camPos, _camFront, _camUp;

const unsigned POINT_SIZE = 5.0f;
const float PARTICLE_QUAD_SIZE = 0.2;
const unsigned NR_EMITTERS = 100;
const float PARTICLE_START_SPEED_INCREMENT = 0.01f;
float _tLastEmission = 0.0;
unsigned _nrLiveParticles = 0;
bool _isSimulationOn = false;
Emitter** _emitters;
GLuint _particleShader;
char _texParticlePath[128];
GLuint _texParticle;
float _particleStartSpeedToAdd = 0.0f;

const unsigned NR_ATTRACTORS = 2;
const float ATTRACTOR_QUAD_SIZE = 1.0;
const float ATTRACTOR_MASS_INCREMENT = 0.1;
Attractor** _attractors;
GLuint _attractorShader;
GLuint _texAttractor, _attractorVao, _bufAttractorPos;
float _attractorMass;

const float AXIS_SIZE = 100.0f;
bool _shouldDrawAxes = false;
GLuint _axisVao, _axisShader, _bufAxisPos, _bufAxisColor;

const float BASE_FONT_SIZE = 15.0f * RESOLUTION_SCALE;
const float LINE_HEIGHT = 30.0f * RESOLUTION_SCALE;
const float GUI_MARGIN = 15.0f * RESOLUTION_SCALE;
bool _shouldDrawGui = true;
GLuint _characterVao, _characterShader, _bufCharacterPos, _bufCharacterTexcoord,
  _bufCharacterColor;
FT_Face _faceCopy;
Character _charsCopy[128];

/*
For the given shader, sets the `view` and `proj` uniforms to their current
values in the program.
This is for orthographic projection.
*/
void setOrthoView(GLuint shader) {
  GLuint loc;
  loc = glGetUniformLocation(shader, "proj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(_orthoProj));
}

/*
For the given shader, sets the `view` and `proj` uniforms to their current
values in the program.
This is for perspective projection.
*/
void setPerspectiveView(GLuint shader) {
  GLuint loc;
  loc = glGetUniformLocation(shader, "view");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(_view));
  loc = glGetUniformLocation(shader, "proj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(_proj));
  loc = glGetUniformLocation(shader, "camPos");
  glUniform3fv(loc, 1, glm::value_ptr(_camPos));
  loc = glGetUniformLocation(shader, "camUp");
  glUniform3fv(loc, 1, glm::value_ptr(_camUp));
}

/*
Sets the isPoint property in the selected shader.
*/
void setIsPoint(GLuint shader) {
  GLint isPoint = 0;
  if (_drawingMode == POINT) {
    isPoint = 1;
  }
  GLuint loc;
  loc = glGetUniformLocation(shader, "isPoint");
  glUniform1i(loc, isPoint);
}

/*
Draws the X, Y, Z axes.
*/
void drawAxes(void) {
  glUseProgram(_axisShader);
  setPerspectiveView(_axisShader);
  glBindVertexArray(_axisVao);
  glDrawArrays(GL_LINES, 0, 12);
}

/*
Draws a string of characters with a given face and character set.
[logl-text-rendering]
*/
void drawString(
  FT_Face face, Character chars[], std::string text, GLfloat x, GLfloat y,
  GLfloat scale, glm::vec3 color
) {
  std::string::const_iterator iChar;
  for (iChar = text.begin(); iChar != text.end(); iChar++) {
    Character ch = chars[(unsigned char)*iChar];

    GLfloat xpos = x + ch.bearing.x * scale;
    GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

    GLfloat w = ch.size.x * scale;
    GLfloat h = ch.size.y * scale;

    GLfloat vertices[6][7] = {
      {xpos,     ypos + h,  0.0f, 0.0f, color[0], color[1], color[2]},
      {xpos,     ypos,      0.0f, 1.0f, color[0], color[1], color[2]},
      {xpos + w, ypos,      1.0f, 1.0f, color[0], color[1], color[2]},
      {xpos,     ypos + h,  0.0f, 0.0f, color[0], color[1], color[2]},
      {xpos + w, ypos,      1.0f, 1.0f, color[0], color[1], color[2]},
      {xpos + w, ypos + h,  1.0f, 0.0f, color[0], color[1], color[2]}
    };

    glBindTexture(GL_TEXTURE_2D, ch.id);

    // Buffer orphaning is used here to improve performance [buffer-orphaning]
    glBindBuffer(GL_ARRAY_BUFFER, _bufCharacterPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, vertices, GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, _bufCharacterTexcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, vertices, GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, _bufCharacterColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, vertices, GL_DYNAMIC_DRAW
    );

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Bitshift by 6 to get value in pixels (2^6 = 64)
    x += (ch.advance >> 6) * scale;
  }
}

/*
Draws the GUI text.
*/
void drawCharacters(void) {
  glUseProgram(_characterShader);
  setOrthoView(_characterShader);
  glBindVertexArray(_characterVao);
  glActiveTexture(GL_TEXTURE0);

  float time = glfwGetTime();

  float currY = _windowHeight - LINE_HEIGHT;
  char currString[128];
  glm::vec3 color = glm::vec3(0.1f, 0.1f, 0.1f);

  snprintf(currString, sizeof(currString), "(particle-system");
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString),
    " (t %f)", time
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString),
    " (simulation %s)", _isSimulationOn ? "on" : "off"
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString), " (fps %f)", _fpsLast
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString),
    " (nr-live-particles %u)", _nrLiveParticles
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString),
    " (attractor-mass %f)", _attractors[0]->mass
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString),
    " (init-speed %f %f %f)",
    _emitters[0]->partSpeed.x, _emitters[0]->partSpeed.y,
    _emitters[0]->partSpeed.z
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;

  snprintf(
    currString, sizeof(currString),
    " (mode %s))", (_drawingMode == QUAD) ? "QUAD" : "POINT"
  );
  drawString(_faceCopy, _charsCopy, currString, GUI_MARGIN, currY, 1.0f, color);
  currY -= LINE_HEIGHT;
}

/*
Draws the particles for a single emitter.
*/
unsigned drawEmitter(Emitter* emitter) {
  glBindVertexArray(emitter->vao);

  std::vector<Particle> particles = emitterGetLiveParticles(emitter);

  // Buffer orphaning is used here to improve performance [buffer-orphaning]
  glBindBuffer(GL_ARRAY_BUFFER, emitter->bufPartPos);
  glBufferData(GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), NULL, GL_STREAM_DRAW);
  glBufferData(
    GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), particles.data(), GL_STREAM_DRAW
  );
  glBindBuffer(GL_ARRAY_BUFFER, emitter->bufPartColor);
  glBufferData(GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), NULL, GL_STREAM_DRAW);
  glBufferData(
    GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), particles.data(), GL_STREAM_DRAW
  );
  glBindBuffer(GL_ARRAY_BUFFER, emitter->bufPartTtl);
  glBufferData(GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), NULL, GL_STREAM_DRAW);
  glBufferData(
    GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), particles.data(), GL_STREAM_DRAW
  );

  if (_drawingMode == POINT) {
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.size());
  } else if (_drawingMode == QUAD) {
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());
  }

  return particles.size();
}

/*
Draws the particles for each emitter.
*/
void drawParticles(void) {
  glUseProgram(_particleShader);
  setPerspectiveView(_particleShader);
  setIsPoint(_particleShader);

  glBindTexture(GL_TEXTURE_2D, _texParticle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  _nrLiveParticles = 0;
  for (int iEmitter = 0; iEmitter < NR_EMITTERS; iEmitter++) {
    Emitter* emitter = _emitters[iEmitter];
    unsigned emitterNrParticles = drawEmitter(emitter);
    _nrLiveParticles += emitterNrParticles;
  }
}

/*
Draws the attractors.
*/
void drawAttractors(void) {
  glUseProgram(_attractorShader);
  setPerspectiveView(_attractorShader);
  setIsPoint(_attractorShader);

  glBindTexture(GL_TEXTURE_2D, _texAttractor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindVertexArray(_attractorVao);

  std::vector<Attractor> attractors = attractorDereference(_attractors, NR_ATTRACTORS);

  // Buffer orphaning is used here to improve performance [buffer-orphaning]
  glBindBuffer(GL_ARRAY_BUFFER, _bufAttractorPos);
  glBufferData(GL_ARRAY_BUFFER, NR_ATTRACTORS * sizeof(Attractor), NULL, GL_STREAM_DRAW);
  glBufferData(
    GL_ARRAY_BUFFER, NR_ATTRACTORS * sizeof(Attractor), attractors.data(), GL_STREAM_DRAW
  );

  if (_drawingMode == POINT) {
    glDrawArraysInstanced(GL_POINTS, 0, 1, attractors.size());
  } else if (_drawingMode == QUAD) {
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, attractors.size());
  }
}

/*
Called on each frame to draw things.
*/
void draw(void) {
  glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (_shouldDrawAxes) { drawAxes(); }
  drawAttractors();
  drawParticles();
  if (_shouldDrawGui) { drawCharacters(); }
}

/*
Actually move the camera around.
*/
void updateCameraPosition(double x, double y, bool programmatic) {
  float pitch, yaw;

  controlsUpdateMouselook(x, y, false, &pitch, &yaw);

  glm::vec3 front;
  front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
  front.y = sin(glm::radians(pitch));
  front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
  _camFront = glm::normalize(front);
}

/*
Called when the mouse is moved.
*/
void processMouse(GLFWwindow* window, double x, double y) {
  updateCameraPosition(x, y, false);
}

/*
Updates the key buffer when a key action is performed.
*/
void processKeypress(GLFWwindow* window, int key, int scancode, int action, int mods) {
  controlsUpdateKeys(window, key, scancode, action, mods);
}

/*
Puts the attractors in their starting positions.
*/
void makeStartingAttractors(void) {
  _attractors[0] = attractorMake(
    glm::vec3(0.0f, 0.0f, 20.0f),
    glm::vec3(0.0f, -0.1f, 0.0f),
    _attractorMass
  );
  _attractors[1] = attractorMake(
    glm::vec3(10.0f, 0.0f, -20.0f),
    glm::vec3(0.0f, 0.1f, 0.0f),
    _attractorMass
  );
}

/*
Actually update the camera's position depending on which keys are currently
pressed.
*/
void performMovement() {
  if (controlsIsKeyDown(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(_window, GL_TRUE);
  }
  if(controlsIsKeyDown(GLFW_KEY_W)) {
    _camPos += _camSpeed * _camFront;
  }
  if(controlsIsKeyDown(GLFW_KEY_S)) {
    _camPos -= _camSpeed * _camFront;
  }
  if(controlsIsKeyDown(GLFW_KEY_D)) {
    _camPos += glm::normalize(glm::cross(_camFront, _camUp)) * _camSpeed;
  }
  if(controlsIsKeyDown(GLFW_KEY_A)) {
    _camPos -= glm::normalize(glm::cross(_camFront, _camUp)) * _camSpeed;
  }
  if(controlsIsKeyDown(GLFW_KEY_C)) {
    _camPos -= _camUp * _camSpeed;
  }
  if(controlsIsKeyDown(GLFW_KEY_SPACE)) {
    _camPos += _camUp * _camSpeed;
  }

  if(controlsDidKeyComeDown(GLFW_KEY_ENTER)) {
    _isSimulationOn = !_isSimulationOn;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_X)) {
    _shouldDrawAxes = !_shouldDrawAxes;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_TAB)) {
    _shouldDrawGui = !_shouldDrawGui;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_R)) {
    makeStartingAttractors();
  }
  if(controlsDidKeyComeDown(GLFW_KEY_1)) {
    _attractorMass -= ATTRACTOR_MASS_INCREMENT;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_2)) {
    _attractorMass += ATTRACTOR_MASS_INCREMENT;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_3)) {
    _particleStartSpeedToAdd -= PARTICLE_START_SPEED_INCREMENT;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_4)) {
    _particleStartSpeedToAdd += PARTICLE_START_SPEED_INCREMENT;
  }
  if(controlsDidKeyComeDown(GLFW_KEY_E)) {
    for (int iEmitter = 0; iEmitter < NR_EMITTERS; iEmitter++) {
      emitterEmitNext(_emitters[iEmitter]);
    }
  }
}

/*
Initialise fonts and load data for every character. [logl-text-rendering]
*/
void initFonts(void) {
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    utilLogError("[initFonts] Could not init FreeType");
    exit(EXIT_FAILURE);
  }

  if (FT_New_Face(ft, "fonts/LetterGothicMTStd.ttf", 0, &_faceCopy)) {
    utilLogError("[initFonts] Could not load font");
    exit(EXIT_FAILURE);
  }

  FT_Set_Pixel_Sizes(_faceCopy, 0, BASE_FONT_SIZE);

  // Disable byte-alignment restriction [logl-text-rendering]
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (GLubyte code = 0; code < 128; code++) {
    if (FT_Load_Char(_faceCopy, code, FT_LOAD_RENDER)) {
      utilLogError("[initFonts] Could not load glyph %d", code);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RED, _faceCopy->glyph->bitmap.width,
      _faceCopy->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
      _faceCopy->glyph->bitmap.buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character = {
      texture,
      glm::ivec2(_faceCopy->glyph->bitmap.width, _faceCopy->glyph->bitmap.rows),
      glm::ivec2(_faceCopy->glyph->bitmap_left, _faceCopy->glyph->bitmap_top),
      _faceCopy->glyph->advance.x
    };
    _charsCopy[code] = character;
  }
}

/*
Initialise OpenGL, GLFW and GLEW. Create the window and context.
*/
void initGraphics(void) {
  glfwSetErrorCallback(utilLogGraphicsError);
  if (!glfwInit()) {
    utilLogError("[init] Could not init GLFW");
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

  _windowWidth = mode->width * RESOLUTION_SCALE;
  _windowHeight = mode->height * RESOLUTION_SCALE;
  _aspectRatio = _windowWidth / _windowHeight;
  strcpy(_windowTitle, "Particles");

  _window = glfwCreateWindow(
    _windowWidth, _windowHeight, _windowTitle, primaryMonitor, NULL
  );
  if (!_window) {
    utilLogError("[init] Could not create window");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(_window);
  glfwSwapInterval(1);

  glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(_window, processMouse);
  glfwSetKeyCallback(_window, processKeypress);
  glPointSize(POINT_SIZE);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  assert(err == GLEW_OK);
  assert(GLEW_VERSION_2_1);

  /* glEnable(GL_DEPTH_TEST); */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/*
Load shader program and VAO for the axes.
*/
void initAxes(void) {
  GLuint loc;
  glGenVertexArrays(1, &_axisVao);
  glBindVertexArray(_axisVao);
  _axisShader = utilMakeShaderProgramWithPaths("axis.vert", "axis.frag");

  static const GLfloat axisData[] = {
  //x          y          z          r     g     b
    0.0f,      0.0f,      0.0f,      1.0f, 0.0f, 0.0f,
    AXIS_SIZE, 0.0f,      0.0f,      1.0f, 0.0f, 0.0f,
    0.0f,      0.0f,      0.0f,      0.0f, 1.0f, 0.0f,
    0.0f,      AXIS_SIZE, 0.0f,      0.0f, 1.0f, 0.0f,
    0.0f,      0.0f,      0.0f,      0.0f, 0.0f, 1.0f,
    0.0f,      0.0f,      AXIS_SIZE, 0.0f, 0.0f, 1.0f,
  };

  glGenBuffers(1, &_bufAxisPos);
  glBindBuffer(GL_ARRAY_BUFFER, _bufAxisPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW);
  loc = glGetAttribLocation(_axisShader, "pos");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    6 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat))
  );

  glGenBuffers(1, &_bufAxisColor);
  glBindBuffer(GL_ARRAY_BUFFER, _bufAxisColor);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW);
  loc = glGetAttribLocation(_axisShader, "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))
  );
}

/*
Load shader program and VAO for the characters.
*/
void initCharacters(void) {
  GLuint loc;
  glGenVertexArrays(1, &_characterVao);
  glBindVertexArray(_characterVao);
  _characterShader = utilMakeShaderProgramWithPaths("character.vert", "character.frag");

  glGenBuffers(1, &_bufCharacterPos);
  glBindBuffer(GL_ARRAY_BUFFER, _bufCharacterPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
  loc = glGetAttribLocation(_characterShader, "pos");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 2, GL_FLOAT, GL_FALSE,
    7 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat))
  );

  glGenBuffers(1, &_bufCharacterTexcoord);
  glBindBuffer(GL_ARRAY_BUFFER, _bufCharacterTexcoord);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
  loc = glGetAttribLocation(_characterShader, "texcoord");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 2, GL_FLOAT, GL_FALSE,
    7 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat))
  );

  glGenBuffers(1, &_bufCharacterColor);
  glBindBuffer(GL_ARRAY_BUFFER, _bufCharacterColor);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
  loc = glGetAttribLocation(_characterShader, "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    7 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat))
  );
}

/*
Load VAO and buffers and all that for an emitter.
*/
void initEmitter(Emitter* emitter) {
  GLuint loc;

  static const GLfloat particleVerticesPoint[] = {
  //x      y      z     texx  texy
    0.0f,  0.0f,  0.0f, -1.0f, 1.0f,
  };

  static const GLfloat particleVerticesQuad[] = {
  //x                    y                    z     texx  texy
    -PARTICLE_QUAD_SIZE, -PARTICLE_QUAD_SIZE, 0.0f, 0.0f, 0.0f,
     PARTICLE_QUAD_SIZE, -PARTICLE_QUAD_SIZE, 0.0f, 1.0f, 0.0f,
    -PARTICLE_QUAD_SIZE,  PARTICLE_QUAD_SIZE, 0.0f, 0.0f, 1.0f,
     PARTICLE_QUAD_SIZE,  PARTICLE_QUAD_SIZE, 0.0f, 1.0f, 1.0f,
  };

  int imgWidth = 200, imgHeight = 200;
  _texParticle = texutilLoadPng(_texParticlePath, &imgWidth, &imgHeight);

  glGenVertexArrays(1, &emitter->vao);
  glBindVertexArray(emitter->vao);

  // base
  GLuint bufParticleBase;
  glGenBuffers(1, &bufParticleBase);
  glBindBuffer(GL_ARRAY_BUFFER, bufParticleBase);
  if (_drawingMode == POINT) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(particleVerticesPoint), particleVerticesPoint, GL_STATIC_DRAW
    );
  } else if (_drawingMode == QUAD) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(particleVerticesQuad), particleVerticesQuad, GL_STATIC_DRAW
    );
  }
  loc = glGetAttribLocation(_particleShader, "base");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 0);

  // texcoord
  GLuint bufParticleTexcoord;
  glGenBuffers(1, &bufParticleTexcoord);
  glBindBuffer(GL_ARRAY_BUFFER, bufParticleTexcoord);
  if (_drawingMode == POINT) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(particleVerticesPoint), particleVerticesPoint, GL_STATIC_DRAW
    );
  } else if (_drawingMode == QUAD) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(particleVerticesQuad), particleVerticesQuad, GL_STATIC_DRAW
    );
  }
  loc = glGetAttribLocation(_particleShader, "texcoord");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 2, GL_FLOAT, GL_FALSE,
    5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 0);

  // pos
  glGenBuffers(1, &emitter->bufPartPos);
  glBindBuffer(GL_ARRAY_BUFFER, emitter->bufPartPos);
  glBufferData(GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), NULL, GL_STREAM_DRAW);
  loc = glGetAttribLocation(_particleShader, "pos");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    sizeof(Particle), (GLvoid*)(0 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 1);

  // color
  glGenBuffers(1, &emitter->bufPartColor);
  glBindBuffer(GL_ARRAY_BUFFER, emitter->bufPartColor);
  glBufferData(GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), NULL, GL_STREAM_DRAW);
  loc = glGetAttribLocation(_particleShader, "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    sizeof(Particle), (GLvoid*)(6 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 1);

  // ttl
  glGenBuffers(1, &emitter->bufPartTtl);
  glBindBuffer(GL_ARRAY_BUFFER, emitter->bufPartTtl);
  glBufferData(GL_ARRAY_BUFFER, emitter->nrParticles * sizeof(Particle), NULL, GL_STREAM_DRAW);
  loc = glGetAttribLocation(_particleShader, "ttl");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 1, GL_UNSIGNED_INT, GL_FALSE,
    sizeof(Particle), (GLvoid*)(7 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 1);
}

/*
Load shader program and VAO for the particles.
*/
void initParticles(void) {
  _particleShader = utilMakeShaderProgramWithPaths("particle.vert", "particle.frag");
  _emitters = (Emitter**)malloc(sizeof(Emitter*) * NR_EMITTERS);

  for (int iEmitter = 0; iEmitter < NR_EMITTERS; iEmitter++) {
    _emitters[iEmitter] = emitterMake(
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(
        utilRandRange(0.0f, 0.05f),
        utilRandRange(0.1f, 0.1f),
        utilRandRange(0.0f, 0.05f)
      ),
      glm::vec3(1.0f, 0.0f, 0.5f),
      0, 2000, 500
    );
    initEmitter(_emitters[iEmitter]);
  }
}

/*
Load shader program and VAO for the attractors.
*/
void initAttractors(void) {
  _attractorShader = utilMakeShaderProgramWithPaths("attractor.vert", "attractor.frag");
  _attractors = (Attractor**)malloc(sizeof(Attractor*) * NR_ATTRACTORS);

  makeStartingAttractors();

  GLuint loc;

  static const int szAttractorStruct = 7 * sizeof(float);

  static const GLfloat attractorVerticesPoint[] = {
  //x      y      z     texx  texy
    0.0f,  0.0f,  0.0f, 0.0f, 0.0f,
  };

  static const GLfloat attractorVerticesQuad[] = {
  //x           y           z     texx  texy
    -ATTRACTOR_QUAD_SIZE, -ATTRACTOR_QUAD_SIZE, 0.0f, 0.0f, 0.0f,
     ATTRACTOR_QUAD_SIZE, -ATTRACTOR_QUAD_SIZE, 0.0f, 1.0f, 0.0f,
    -ATTRACTOR_QUAD_SIZE,  ATTRACTOR_QUAD_SIZE, 0.0f, 0.0f, 1.0f,
     ATTRACTOR_QUAD_SIZE,  ATTRACTOR_QUAD_SIZE, 0.0f, 1.0f, 1.0f,
  };

  int imgWidth = 200, imgHeight = 200;
  _texAttractor = texutilLoadPng("tex/blackdot.png", &imgWidth, &imgHeight);

  glGenVertexArrays(1, &_attractorVao);
  glBindVertexArray(_attractorVao);

  // base
  GLuint bufAttractorBase;
  glGenBuffers(1, &bufAttractorBase);
  glBindBuffer(GL_ARRAY_BUFFER, bufAttractorBase);
  if (_drawingMode == POINT) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(attractorVerticesPoint), attractorVerticesPoint, GL_STATIC_DRAW
    );
  } else if (_drawingMode == QUAD) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(attractorVerticesQuad), attractorVerticesQuad, GL_STATIC_DRAW
    );
  }
  loc = glGetAttribLocation(_attractorShader, "base");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 0);

  // texcoord
  GLuint bufAttractorTexcoord;
  glGenBuffers(1, &bufAttractorTexcoord);
  glBindBuffer(GL_ARRAY_BUFFER, bufAttractorTexcoord);
  if (_drawingMode == POINT) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(attractorVerticesPoint), attractorVerticesPoint, GL_STATIC_DRAW
    );
  } else if (_drawingMode == QUAD) {
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(attractorVerticesQuad), attractorVerticesQuad, GL_STATIC_DRAW
    );
  }
  loc = glGetAttribLocation(_attractorShader, "texcoord");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 2, GL_FLOAT, GL_FALSE,
    5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 0);

  // pos
  glGenBuffers(1, &_bufAttractorPos);
  glBindBuffer(GL_ARRAY_BUFFER, _bufAttractorPos);
  glBufferData(GL_ARRAY_BUFFER, NR_ATTRACTORS * sizeof(Attractor), 0, GL_STREAM_DRAW);
  loc = glGetAttribLocation(_attractorShader, "pos");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 3, GL_FLOAT, GL_FALSE,
    szAttractorStruct, (GLvoid*)(0 * sizeof(float))
  );
  glVertexAttribDivisor(loc, 1);

  _attractorMass = 1.0f;
}

/*
Set the starting view position.
*/
void initView(void) {
  _camPos = glm::vec3(20.0f, 50.0f, 20.0f);
  _camFront = glm::vec3(0.0f, 0.0f, 0.0f);
  _camUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

/*
Initialise everything: graphics, view, data.
*/
void init(void) {
  if (isAlpacaModeOn) {
    strcpy(_texParticlePath, "tex/alpaca.png");
  } else {
    strcpy(_texParticlePath, "tex/whitedot.png");
  }

  initGraphics();
  utilLogInfo("[init] Graphics initialised with code: %d\n", glGetError());

  initView();
  updateCameraPosition(0.0f, 0.0f, true);

  soundInit();
  initFonts();

  initParticles();
  initAttractors();
  initAxes();
  initCharacters();
  utilLogInfo("[init] Initialised with code: %d\n", glGetError());
}

/*
Update the view and projection's attributes based on the camera parameters.
*/
void updateView() {
  _view = glm::lookAt(_camPos, _camPos + _camFront, _camUp);
  _proj = glm::perspective(glm::radians(45.0f), _aspectRatio, _zNear, _zFar);
  _orthoProj = glm::ortho(0.0f, _windowWidth, 0.0f, _windowHeight);
}

/*
Calculate the time between frames.
*/
void calculateTDelta() {
  GLfloat tCurrentFrame = glfwGetTime();
  _tDeltaLastFrame = tCurrentFrame - _tLastFrame;
  _tLastFrame = tCurrentFrame;
}

/*
Calculate and display the number of frames per second.
*/
void calculateFps() {
  _fpsNrFrames++;
  GLfloat time = glfwGetTime() * 1000.0f;

  if (time - _fpsLastTime > 1000.0f) {
    _fpsLast = _fpsNrFrames * 1000.0f / (time - _fpsLastTime);
    _fpsLastTime = time;
    _fpsNrFrames = 0.0f;
  }
}

/*
Scales the camera speed according to the current time between frames.
*/
void adjustCameraSpeed() {
  _camSpeed = CAM_BASE_SPEED * _tDeltaLastFrame;
}

/*
On each frame, listen for events, update the camera parameters, actually
draw stuff, and swap buffers.
*/
void tick(void) {
  calculateTDelta();
  calculateFps();
  adjustCameraSpeed();

  glfwPollEvents();
  performMovement();

  // Play an occasional chord
  static const unsigned chordInterval =
    _tDeltaLastFrame * 1500 + (unsigned)utilRandRange(100.0f, 200.0f);
  if (_nrFrame % chordInterval == 0) {
    unsigned soundIndex = (unsigned)utilRandRange(10.0f, 11.99f);
    soundPlay(soundIndex, 100);
  }

  if (_isSimulationOn) {
    physicsTransform(
      _emitters, NR_EMITTERS, _attractors, NR_ATTRACTORS, _attractorMass,
      _particleStartSpeedToAdd
    );
    _particleStartSpeedToAdd = 0.0f;
  }

  updateView();
  draw();

  glfwSwapBuffers(_window);

  _nrFrame++;
}

/*
Deallocate everything we've made.
*/
void cleanup(void) {
  glfwDestroyWindow(_window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

/*
Entry point.
*/
int main(void) {
  srand(time(NULL));

  init();
  while (!glfwWindowShouldClose(_window)) {
    tick();
  }
  cleanup();
}
