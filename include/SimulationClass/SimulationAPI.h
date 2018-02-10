#ifndef SIMULATIONAPI_H
#define SIMULATIONAPI_H

#include <iostream>
#include "dllexport.h"
#include "SimulationClass\PreviousSimulation.h"
#include "StandaloneTools\numericaltools.h"
#include "ErrorHandling\simExceptionMacros.h"

///One liner functions
DLLEXPORT double   getSimulationTimeAPI(Simulation* simulation);
DLLEXPORT double   getDtAPI(Simulation* simulation);
DLLEXPORT double   getSimMinAPI(Simulation* simulation);
DLLEXPORT double   getSimMaxAPI(Simulation* simulation);
DLLEXPORT void     incrementSimulationTimeByDtAPI(Simulation* simulation);
DLLEXPORT int      getNumberOfParticleTypesAPI(Simulation* simulation);
DLLEXPORT int      getNumberOfParticlesAPI(Simulation* simulation, int partInd);
DLLEXPORT int      getNumberOfAttributesAPI(Simulation* simulation, int partInd);
DLLEXPORT const char* getParticleNameAPI(Simulation* simulation, int partInd);
DLLEXPORT const char* getSatelliteNameAPI(Simulation* simulation, int satInd);
DLLEXPORT bool     areResultsPreparedAPI(Simulation* simulation);
DLLEXPORT LogFile* getLogFilePointerAPI(Simulation* simulation);

//Pointer one liners
DLLEXPORT double* getPointerToParticleAttributeArrayAPI(Simulation* simulation, int partIndex, int attrIndex, bool originalData);

//Field tools
DLLEXPORT double getBFieldAtSAPI(Simulation* simulation, double z, double time);
DLLEXPORT double getEFieldAtSAPI(Simulation* simulation, double z, double time);

//Mu<->VPerp Functions
DLLEXPORT void convertParticleVPerpToMuAPI(Simulation* simulation, int partInd);
DLLEXPORT void convertParticleMuToVPerpAPI(Simulation* simulation, int partInd);

//Simulation Management Function Wrappers
DLLEXPORT void initializeSimulationAPI(Simulation* simulation);
DLLEXPORT void copyDataToGPUAPI(Simulation* simulation);
DLLEXPORT void iterateSimulationAPI(Simulation* simulation, int numberOfIterations, int itersBtwCouts);
DLLEXPORT void copyDataToHostAPI(Simulation* simulation);
DLLEXPORT void freeGPUMemoryAPI(Simulation* simulation);
DLLEXPORT void prepareResultsAPI(Simulation* simulation, bool normalizeToRe);

//Satellite functions
DLLEXPORT void    createSatelliteAPI(Simulation* simulation, int particleInd, double altitude, bool upwardFacing, const char* name);
DLLEXPORT int     getNumberOfSatellitesAPI(Simulation* simulation);
DLLEXPORT int     getSatNumOfDetectedPartsAPI(Simulation* simulation, int satIndex);
DLLEXPORT double* getSatelliteDataPointersAPI(Simulation* simulation, int satelliteInd, int attributeInd);
DLLEXPORT void    writeSatelliteDataToCSVAPI(Simulation* simulation);

DLLEXPORT void    createParticleTypeAPI(Simulation* simulation, const char* name, const char* attrNames, double mass, double charge, long numParts, int posDims, int velDims, double normFactor, const char* loadFileDir="");
DLLEXPORT Simulation* loadCompletedSimDataAPI(const char* fileDir, const char* bFieldModel, const char* eFieldElems, const char* partNames, const char* satNames);

DLLEXPORT Simulation* createSimulationAPI(double dt, double simMin, double simMax, double ionT, double magT, const char* rootdir);
DLLEXPORT void setupNormalSimulationAPI(Simulation* sim, int numParts, const char* loadFileDir);
DLLEXPORT void runNormalSimulationAPI(Simulation* sim, int iterations, int printEvery);
DLLEXPORT void terminateSimulationAPI(Simulation* simulation);
DLLEXPORT void setBFieldModelAPI(Simulation* sim, const char* modelName, double arg1);

#endif//end if for header guard