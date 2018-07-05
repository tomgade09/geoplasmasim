#include "API/utilsAPI.h"

#include "utils/string.h"
#include "ErrorHandling/simExceptionMacros.h"

using utils::string::strToStrVec;

//ParticleDistribution functions
DLLEXP_EXTC PD* createParticleDistributionAPI(const char* saveFolder, const char* attrNames, const char* particleName, double mass)
{
	SIM_API_EXCEP_CHECK(
	std::string save{ saveFolder };
	std::vector<std::string> attrs{ strToStrVec(attrNames) };
	std::string part{ particleName };

	PD* ret;
	if (save == "" && attrs.size() == 0 && part == "" && mass == 0.0)
		ret = new PD();
	else
		ret = new PD(save, attrs, part, mass);

	return ret;
	);
}

DLLEXP_EXTC void addPDEnergyRangeAPI(PD* pd, int energyBins, double Emin, double Emax, bool logE) {
	SIM_API_EXCEP_CHECK(pd->addEnergyRange(energyBins, Emin, Emax, logE)); }

DLLEXP_EXTC void addPDPitchRangeAPI(PD* pd, int pitchBins, double PAmin, double PAmax, bool midBin) {
	SIM_API_EXCEP_CHECK(pd->addPitchRange(pitchBins, PAmin, PAmax, midBin)); }

DLLEXP_EXTC void generatePDAPI(PD* pd, double s_ion, double s_mag) {
	SIM_API_EXCEP_CHECK(pd->generate(s_ion, s_mag)); }

DLLEXP_EXTC void padExtraPDAttrsAPI(PD* pd)
{
	SIM_API_EXCEP_CHECK(
		int attrssize{ (int)pd->data().size() };
		int partsize{ (int)pd->data().at(0).size() };
		std::vector<double> zeroes(partsize);
		for (int iii = 0; iii < attrssize - 3; iii++)
			pd->setattr(zeroes, 3 + iii);
	);
}

DLLEXP_EXTC void writePDAPI(PD* pd) {
	delete pd; }


//DistributionFromDisk functions
DLLEXP_EXTC DFD* loadDistributionFromDiskAPI(const char* name, const char* loadFolder, const char* attrNames, const char* particleName) {
	SIM_API_EXCEP_CHECK(return new DFD(name, loadFolder, particleName, strToStrVec(attrNames))); }

DLLEXP_EXTC const double* DistFromDiskDataAPI(DFD* dfd, int attrInd) {
	SIM_API_EXCEP_CHECK(return dfd->data().at(attrInd).data()); }

DLLEXP_EXTC void DistFromDiskPrintAPI(DFD* dfd, int at) {
	SIM_API_EXCEP_CHECK(dfd->print(at)); }

DLLEXP_EXTC void DistFromDiskPrintDiffAPI(DFD* dfd_this, DFD* dfd_other, int at) {
	SIM_API_EXCEP_CHECK(dfd_this->printdiff(*dfd_other, at)); }

DLLEXP_EXTC void DistFromDiskZeroesAPI(DFD* dfd) {
	SIM_API_EXCEP_CHECK(dfd->zeroes()); }

DLLEXP_EXTC void DistFromDiskCompareAPI(DFD* dfd_this, DFD* dfd_other) {
	SIM_API_EXCEP_CHECK(dfd_this->compare(*dfd_other)); }

DLLEXP_EXTC void deleteDistFromDiskAPI(DFD* dfd) {
	delete dfd; }