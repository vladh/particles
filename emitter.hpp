#ifndef EMITTER_H
#define EMITTER_H

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

#include "particle.hpp"

typedef struct Emitter {
  glm::vec3 pos, speed;
  glm::vec3 partSpeed, partColor;
  unsigned partTtl, partSpawnTtl;
  Particle **particles;
  unsigned nrParticles;
  GLuint vao, bufPartPos, bufPartColor, bufPartTtl;
} Emitter;

/*
Creates a heap-allocated emitter with the given properties.
*/
Emitter* emitterMake(
  glm::vec3 pos, glm::vec3 speed, glm::vec3 partSpeed,
  glm::vec3 partColor, unsigned partTtl, unsigned partSpawnTtl,
  unsigned nrParticles
);

/*
Prints an emitter with its properties and particles to stdout.
*/
void emitterPrint(Emitter* emitter);

/*
Emits the particle at the specified index from the emitter.
*/
void emitterEmitParticle(Emitter* emitter, unsigned index);

/*
Emits the next available particle from the emitter.
*/
void emitterEmitNext(Emitter* emitter);

/*
Gets a vector of the live particles from a given emitter.
*/
std::vector<Particle> emitterGetLiveParticles(Emitter* emitter);

#endif
