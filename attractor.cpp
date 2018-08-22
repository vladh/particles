#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <vector>

#include "attractor.hpp"
#include "util.hpp"

Attractor* attractorMake(glm::vec3 pos, glm::vec3 speed, float mass) {
  Attractor* attractor = (Attractor*)malloc(sizeof(Attractor));
  attractor->pos = pos;
  attractor->speed = speed;
  attractor->mass = mass;
  return attractor;
}

void attractorPrint(Attractor* attractor) {
  utilLogInfo(
    "(attractor (p %f %f %f) (s %f %f %f) (m %f))",
    attractor->pos.x, attractor->pos.y, attractor->pos.z,
    attractor->speed.x, attractor->speed.y, attractor->speed.z,
    attractor->mass
  );
}

std::vector<Attractor> attractorDereference(
  Attractor** attractorRefs, unsigned nrAttractors
) {
  std::vector<Attractor> attractors;
  for (int iAttractor = 0; iAttractor < nrAttractors; iAttractor++) {
    attractors.push_back(*(attractorRefs[iAttractor]));
  }
  return attractors;
}
