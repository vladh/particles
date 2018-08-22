#ifndef ATTRACTOR_H
#define ATTRACTOR_H

#include <GL/glew.h>
#include <glm/glm.hpp>

typedef struct Attractor {
  glm::vec3 pos, speed;
  float mass;
} Attractor;

/*
Creates a heap-allocated Attractor with the given properties.
*/
Attractor* attractorMake(glm::vec3 pos, glm::vec3 speed, float mass);

/*
Prints an Attractor with its properties and particles to stdout.
*/
void attractorPrint(Attractor* Attractor);

/*
Makes an array of Attractor pointers into a std::vector of Attractors.
*/
std::vector<Attractor> attractorDereference(
  Attractor** attractor, unsigned nrAttractors
);

#endif
