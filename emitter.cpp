#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>

#include "util.hpp"
#include "emitter.hpp"
#include "particle.hpp"
#include "sound.hpp"

Emitter* emitterMake(
  glm::vec3 pos,
  glm::vec3 speed,
  glm::vec3 partSpeed,
  glm::vec3 partColor,
  unsigned partTtl, unsigned partSpawnTtl,
  unsigned nrParticles
) {
  Emitter* emitter = (Emitter*)malloc(sizeof(Emitter));
  emitter->pos = pos;
  emitter->speed = speed;
  emitter->partSpeed = partSpeed;
  emitter->partColor = partColor;
  emitter->partTtl = partTtl;
  emitter->partSpawnTtl = partSpawnTtl;
  emitter->nrParticles = nrParticles;

  emitter->particles = (Particle**)malloc(sizeof(Particle*) * nrParticles);
  for (int iParticle = 0; iParticle < nrParticles; iParticle++) {
    emitter->particles[iParticle] = particleMake(
      pos, partSpeed, partColor, partTtl, false, 0
    );
  }
  return emitter;
}

void emitterPrint(Emitter* emitter) {
  utilLogInfo(
    "(emitter (p %f %f %f) (s %f %f %f) (c %f %f %f) (ttl %u %u) (n %u))",
    emitter->pos.x, emitter->pos.y, emitter->pos.z,
    emitter->speed.x, emitter->speed.y, emitter->speed.z,
    emitter->partColor.r, emitter->partColor.g, emitter->partColor.b,
    emitter->partTtl, emitter->partSpawnTtl,
    emitter->nrParticles
  );
  for (int iParticle = 0; iParticle < emitter->nrParticles; iParticle++) {
    particlePrint(emitter->particles[iParticle]);
  }
}

void emitterEmitParticle(Emitter* emitter, unsigned index) {
  bool isSoundSource = true;
  unsigned soundIndex = 0;

  if (isSoundSource) {
    soundIndex = (unsigned)utilRandRange(100.0f, 159.99f);
  }

  emitter->particles[index] = particleMake(
    emitter->pos, emitter->partSpeed, emitter->partColor, emitter->partTtl,
    isSoundSource, soundIndex
  );
  emitter->particles[index]->ttl = emitter->partSpawnTtl;
}

void emitterEmitNext(Emitter* emitter) {
  unsigned freeIndex;
  bool freeIndexFound = false;
  for (int iParticle = 0; iParticle < emitter->nrParticles; iParticle++) {
    if (emitter->particles[iParticle]->ttl == 0) {
      freeIndex = iParticle;
      freeIndexFound = true;
      break;
    }
  }
  if (!freeIndexFound) { freeIndex = 0; }
  emitterEmitParticle(emitter, freeIndex);
}

std::vector<Particle> emitterGetLiveParticles(Emitter* emitter) {
  std::vector<Particle> particles;
  for (int iParticle = 0; iParticle < emitter->nrParticles; iParticle++) {
    if (emitter->particles[iParticle]->ttl != 0) {
      particles.push_back(*(emitter->particles[iParticle]));
    }
  }
  return particles;
}
