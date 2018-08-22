#include <glm/glm.hpp>

#include "util.hpp"
#include "emitter.hpp"
#include "particle.hpp"
#include "attractor.hpp"
#include "sound.hpp"

const float PARTICLE_MASS = 1.0f;
const float MIN_DIST = 10.0f;
const float MAX_DIST = 1000.0f;
const float ATTRACTOR_SOUND_RANGE = 1.0f;

const bool ATTRACTOR_PHYSICS_ON = true;
const bool PARTICLE_PHYSICS_ON = true;

/*
Calculates the gravitational force between bodies x and y.
*/
glm::vec3 calculateGravitationalForce(
  glm::vec3 xLocation, glm::vec3 yLocation,
  float xMass, float yMass
) {
  const float G = 1;

  glm::vec3 delta = xLocation - yLocation;
  float distance = utilClamp(glm::length(delta), MIN_DIST, MAX_DIST);

  glm::vec3 direction = glm::normalize(delta);

  float strength = (G * xMass * yMass) / (distance * distance);
  glm::vec3 force = direction * strength;
  return force;
}

/*
Transforms a single particle according to a set of attractors.
*/
void transformParticle(
  Emitter* emitter, Particle* particle,
  Attractor** attractors, unsigned nrAttractors
) {
  if (particle->ttl == 0) { return; }

  particle->ttl--;
  if (particle->color.r > 0.0f) { particle->color.r -= 0.0005f; }
  if (particle->color.g > 0.0f) { particle->color.g -= 0.0005f; }
  if (particle->color.b > 0.0f) { particle->color.b -= 0.0005f; }

  if (PARTICLE_PHYSICS_ON) {
    for (unsigned iAttractor = 0; iAttractor < nrAttractors; iAttractor++) {
      Attractor* attractor = attractors[iAttractor];

      glm::vec3 gravForce = calculateGravitationalForce(
        particle->pos, attractor->pos, PARTICLE_MASS, attractor->mass
      );

      glm::vec3 oldPos = particle->pos;

      particle->speed += gravForce;
      particle->pos -= particle->speed;

      float oldDist = glm::length(oldPos - attractor->pos);
      float newDist = glm::length(particle->pos - attractor->pos);
      bool enteredAttractorSoundRange =
        (
          (oldDist > ATTRACTOR_SOUND_RANGE) &&
          (newDist <= ATTRACTOR_SOUND_RANGE)
        ) || (
          (oldDist < ATTRACTOR_SOUND_RANGE) &&
          (newDist >= ATTRACTOR_SOUND_RANGE)
        );

      if (
        particle->isSoundSource && enteredAttractorSoundRange && (
          particle->color.r > 0.0f ||
          particle->color.g > 0.0f ||
          particle->color.b > 0.0f
        )
      ) {
        unsigned volume;
        if (utilRandRange(0.0f, 1.0f) < 0.7f) {
          volume = (unsigned)utilRandRange(0.0f, 10.99f);
        } else {
          volume = (unsigned)utilRandRange(11.0f, 100.99f);
        }
        soundPlay(particle->soundIndex, volume);
      }
    }
  }
}

/*
Transforms an emitter and its particles according to a set of attractors.
*/
void transformEmitter(
  Emitter* emitter, Attractor** attractors, unsigned nrAttractors,
  float particleStartSpeedToAdd
) {
  emitter->partSpeed += particleStartSpeedToAdd;
  if (emitter->partSpeed.x < 0) { emitter->partSpeed.x = 0.0f; }
  if (emitter->partSpeed.y < 0) { emitter->partSpeed.y = 0.0f; }
  if (emitter->partSpeed.z < 0) { emitter->partSpeed.z = 0.0f; }
  for (unsigned iParticle = 0; iParticle < emitter->nrParticles; iParticle++) {
    Particle *particle = emitter->particles[iParticle];
    transformParticle(emitter, particle, attractors, nrAttractors);
  }
}

/*
Transforms an attractor according to its surrounding attractors.
*/
void transformAttractor(
  Attractor* ourAttractor, unsigned iOurAttractor,
  Attractor** attractors, unsigned nrAttractors,
  float attractorMass
) {
  ourAttractor->mass = attractorMass;
  if (ATTRACTOR_PHYSICS_ON) {
    for (unsigned iAttractor = 0; iAttractor < nrAttractors; iAttractor++) {
      if (iAttractor == iOurAttractor) { continue; }
      Attractor* attractor = attractors[iAttractor];

      glm::vec3 gravForce = calculateGravitationalForce(
        ourAttractor->pos, attractor->pos, ourAttractor->mass, attractor->mass
      );
      ourAttractor->speed += gravForce;
      ourAttractor->pos -= ourAttractor->speed;
    }
  }
}

void physicsTransform(
  Emitter** emitters, unsigned nrEmitters,
  Attractor** attractors, unsigned nrAttractors,
  float attractorMass, float particleStartSpeedToAdd
) {
  for (unsigned iEmitter = 0; iEmitter < nrEmitters; iEmitter++) {
    Emitter* emitter = emitters[iEmitter];
    transformEmitter(emitter, attractors, nrAttractors, particleStartSpeedToAdd);
  }
  for (unsigned iAttractor = 0; iAttractor < nrAttractors; iAttractor++) {
    Attractor* attractor = attractors[iAttractor];
    transformAttractor(attractor, iAttractor, attractors, nrAttractors, attractorMass);
  }
}
