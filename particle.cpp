#include <stdio.h>
#include <stdlib.h>

#include "particle.hpp"
#include "util.hpp"

Particle* particleMake(
  glm::vec3 pos, glm::vec3 speed, glm::vec3 color, unsigned ttl,
  bool isSoundSource, unsigned soundIndex
) {
  Particle* particle = (Particle*)malloc(sizeof(Particle));
  particle->pos = pos;
  particle->speed = speed;
  particle->color = color;
  particle->ttl = ttl;
  particle->isSoundSource = isSoundSource;
  particle->soundIndex = soundIndex;
  return particle;
}

void particlePrint(Particle* particle) {
  utilLogInfo(
    "(particle (p %f %f %f) (s %f %f %f) (c %f %f %f) (ttl %u))",
    particle->pos.x, particle->pos.y, particle->pos.z,
    particle->speed.x, particle->speed.y, particle->speed.z,
    particle->color.r, particle->color.g, particle->color.b,
    particle->ttl
  );
}
