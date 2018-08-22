#ifndef PHYSICS_H
#define PHYSICS_H

/*
Performs various physics transformations.
*/
void physicsTransform(
  Emitter** emitters, unsigned nrEmitters,
  Attractor** attractors, unsigned nrAttractors,
  float attractorMass, float particleStartSpeedToAdd
);

#endif
