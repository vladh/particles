#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <SFML/Audio.hpp>

typedef struct Particle {
  glm::vec3 pos, speed, color;
  unsigned ttl;
  bool isSoundSource;
  unsigned soundIndex;
} Particle;

/*
Creates a heap-allocated particle with the given properties.
*/
Particle* particleMake(
  glm::vec3 pos, glm::vec3 speed, glm::vec3 color, unsigned ttl,
  bool isSoundSource, unsigned soundIndex
);

/*
Prints the given particle and its properties to stdout.
*/
void particlePrint(Particle* particles);

#endif
