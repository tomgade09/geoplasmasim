#include "SimulationClass\SimulationAPI.h"

///One liner functions
DLLEXPORT double getSimulationTimeAPI(Simulation* simulation) {
	return simulation->getTime(); }

DLLEXPORT double getDtAPI(Simulation* simulation) {
	return simulation->getdt(); }

DLLEXPORT double getSimMinAPI(Simulation* simulation) {
	return simulation->getSimMin(); }

DLLEXPORT double getSimMaxAPI(Simulation* simulation) {
	return simulation->getSimMax(); }

DLLEXPORT void incrementSimulationTimeByDtAPI(Simulation* simulation) {
	simulation->incTime(); }

DLLEXPORT void setQSPSAPI(Simulation* simulation, double constE) {
	simulation->setQSPS(constE); }

DLLEXPORT int getNumberOfParticleTypesAPI(Simulation* simulation) {
	return static_cast<int>(simulation->getNumberOfParticleTypes()); }

DLLEXPORT int getNumberOfParticlesAPI(Simulation* simulation, int partInd) {
	return static_cast<int>(simulation->getNumberOfParticles(partInd)); }

DLLEXPORT int getNumberOfAttributesAPI(Simulation* simulation, int partInd) {
	return static_cast<int>(simulation->getNumberOfAttributes(partInd)); }

DLLEXPORT bool areResultsPreparedAPI(Simulation* simulation) {
	return simulation->areResultsPrepared(); }

DLLEXPORT LogFile* getLogFilePointerAPI(Simulation* simulation) {
	return simulation->getLogFilePointer(); }


//Pointer one liners
DLLEXPORT double* getPointerToParticleAttributeArrayAPI(Simulation* simulation, int partIndex, int attrIndex, bool originalData) {
	return simulation->getPointerToParticleAttributeArray(partIndex, attrIndex, originalData); }


//Field tools
DLLEXPORT double calculateBFieldAtZandTimeAPI(Simulation* simulation, double z, double time) {
	return simulation->calculateBFieldAtZandTime(z, time); }

DLLEXPORT double calculateEFieldAtZandTimeAPI(Simulation* simulation, double z, double time) {
	return simulation->calculateEFieldAtZandTime(z, time); }


//Mu<->VPerp Functions
DLLEXPORT void convertParticleVPerpToMuAPI(Simulation* simulation, int partInd) {
	simulation->convertVPerpToMu(partInd); }

DLLEXPORT void convertParticleMuToVPerpAPI(Simulation* simulation, int partInd) {
	simulation->convertMuToVPerp(partInd); }


//Simulation Management Function Wrappers
DLLEXPORT void initializeSimulationAPI(Simulation* simulation) {
	simulation->initializeSimulation(); }

DLLEXPORT void copyDataToGPUAPI(Simulation* simulation) {
	simulation->copyDataToGPU(); }

DLLEXPORT void iterateSimulationAPI(Simulation* simulation, int numberOfIterations) {
	simulation->iterateSimulation(numberOfIterations); }

DLLEXPORT void copyDataToHostAPI(Simulation* simulation) {
	simulation->copyDataToHost(); }

DLLEXPORT void freeGPUMemoryAPI(Simulation* simulation) {
	simulation->freeGPUMemory(); }

DLLEXPORT void prepareResultsAPI(Simulation* simulation, bool normalizeToRe) {
	simulation->prepareResults(normalizeToRe); }


//Satellite functions
DLLEXPORT void createSatelliteAPI(Simulation* simulation, int particleInd, double altitude, bool upwardFacing, const char* name) {
	simulation->createTempSat(particleInd, altitude, upwardFacing, name); }

DLLEXPORT int  getNumberOfSatellitesAPI(Simulation* simulation) {
	return static_cast<int>(simulation->getNumberOfSatellites()); }

DLLEXPORT double* getSatelliteDataPointersAPI(Simulation* simulation, int satelliteInd, int attributeInd) {
	return simulation->getSatelliteDataPointers(satelliteInd, attributeInd); }

DLLEXPORT void writeSatelliteDataToCSVAPI(Simulation* simulation) {
	simulation->writeSatelliteDataToCSV(); }


//Particle functions
DLLEXPORT void createParticleTypeAPI(Simulation* simulation, const char* name, const char* attrNames, double mass, double charge, long numParts, int posDims, int velDims, double normFactor, const char* loadFileDir)
{
	std::string nameStr{ name };
	std::string tmp{ attrNames };
	std::vector<std::string> attrNamesVec;

	size_t loc{ 0 };
	while (loc != std::string::npos)
	{
		loc = tmp.find(',');
		attrNamesVec.push_back(tmp.substr(0, loc));
		tmp.erase(0, loc + 1);
		while (tmp.at(0) == ' ')
			tmp.erase(0, 1);
	}

	simulation->createParticleType(nameStr, attrNamesVec, mass, charge, numParts, posDims, velDims, normFactor, loadFileDir);
}