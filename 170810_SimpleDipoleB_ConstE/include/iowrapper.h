#ifndef IOWRAPPER_H
#define IOWRAPPER_H

#include "_simulationvariables.h"
#include "include\fileIO.h"

void writeParticlesToBin(double*** particles, std::string directory);
double*** readParticlesFromBin(std::string filename);

#endif //end header guard