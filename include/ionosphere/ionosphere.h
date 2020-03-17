#ifndef IONOSPHERE_H
#define IONOSPHERE_H

#include <vector>
#include "dlldefines.h"
#include "ionosphere/ionosphereClasses.h"

namespace ionosphere
{
	DLLEXP dEflux_v2D steadyFlux(const EOMSimData& eomdata);

	namespace dEFlux
	{
		DLLEXP dEflux_v2D satellite(const ParticleData& particles, const Bins& satBins, const dNflux_v1D& dNatSat);
		DLLEXP dEflux_v2D backscatr(const EOMSimData& eomdata, const dNflux_v1D& dNatIonsph1D);
		DLLEXP degrees newPA(const degrees PA_init, const tesla B_init, const tesla B_final);
	}

	namespace binning
	{
		DLLEXP dNflux_v2D binParticles(const ParticleData& particles, const Bins& bins, const dNflux_v1D& countPerParticle);
		DLLEXP void       symmetricBins180To360(dEflux_v2D& data, double_v1D& binAngles);
	}

	namespace backscat
	{
		DLLEXP dNflux     johnd_flux(eV E_eval, eV E_incident, dNflux dN_incident);
		DLLEXP dNflux_v2D downwardToBackscatter(const Bins& dist, const dNflux_v2D& dNpointofScatter);
		DLLEXP dNflux_v2D ionsphToSatellite(const EOMSimData& eomdata, const dNflux_v2D& bsCounts);
	}

	namespace multiLevelBS
	{
		DLLEXP dNflux_v2D scatterMain(const EOMSimData& eom, const dNflux_v2D& dNionsphTop);
		DLLEXP dNflux_v2D bsAtLevel(const EOMSimData& eom, const dNflux_v2D& dNionsphTop, double_v2D& pctScatteredAbove, size_t level);
		DLLEXP percent    scatterPct(percent sumCollideAbove, double Z, percm3 p, cm h, eV E, degrees PA);
	}
}

#endif /* !IONOSPHERE_H */