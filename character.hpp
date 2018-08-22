#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

typedef struct Character {
  GLuint id;
  glm::ivec2 size;
  // Top side bearing - the vertical distance from the baseline to the
  // top of the glyph's bbox.
  glm::ivec2 bearing;
  // Memory offset to next glyph
  GLuint advance;
} Character;

#endif
