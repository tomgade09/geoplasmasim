#include "BField/DipoleB.h"

#include "device_launch_parameters.h"
#include "ErrorHandling/cudaErrorCheck.h"
#include "ErrorHandling/cudaDeviceMacros.h"

//setup CUDA kernels
__global__ void setupEnvironmentGPU_DipoleB(BField** this_d, double ILATDeg, double errTol, double ds)
{
	ZEROTH_THREAD_ONLY("setupEnvironmentGPU_DipoleB", (*this_d) = new DipoleB(ILATDeg, errTol, ds));
}

__global__ void deleteEnvironmentGPU_DipoleB(BField** dipoleb)
{
	ZEROTH_THREAD_ONLY("deleteEnvironmentGPU_DipoleB", delete ((DipoleB*)(*dipoleb)));
}

__host__ __device__ DipoleB::DipoleB(double ILATDegrees, double errorTolerance, double ds) :
	BField("DipoleB"), ILATDegrees_m{ ILATDegrees }, ds_m{ ds }, errorTolerance_m{ errorTolerance }
{
	L_m = RADIUS_EARTH / pow(cos(ILATDegrees * RADS_PER_DEG), 2);
	L_norm_m = L_m / RADIUS_EARTH;
	s_max_m = getSAtLambda(ILATDegrees_m);

	#ifndef __CUDA_ARCH__ //host code
	setupEnvironment();
	#endif /* !__CUDA_ARCH__ */
}

__host__ __device__ DipoleB::~DipoleB()
{
	#ifndef __CUDA_ARCH__ //host code
	deleteEnvironment();
	#endif /* !__CUDA_ARCH__ */
}

//B Field related kernels
__host__ __device__ double DipoleB::getSAtLambda(const double lambdaDegrees) const
{
	//double x{ asinh(sqrt(3.0) * sinpi(lambdaDegrees / 180.0)) };
	double sinh_x{ sqrt(3.0) * sinpi(lambdaDegrees / 180.0) };
	double x{ log(sinh_x + sqrt(sinh_x * sinh_x + 1)) }; //trig identity for asinh - a bit faster - asinh(x) == ln(x + sqrt(x*x + 1))

	return (0.5 * L_m / sqrt(3.0)) * (x + 0.25 * (exp(2.0*x)-exp(-2.0*x))); /* L */ //0.25 * (exp(2*x)-exp(-2*x)) == sinh(x) * cosh(x) and is faster
}

__host__ __device__ double DipoleB::getLambdaAtS(const double s) const
{
	double lambda_tmp{ (-ILATDegrees_m / s_max_m) * s + ILATDegrees_m }; //-ILAT / s_max * s + ILAT
	double s_tmp{ s_max_m - getSAtLambda(lambda_tmp) };
	double dlambda{ 1.0 };
	bool   over{ 0 };

	while (abs((s_tmp - s) / s) > errorTolerance_m)
	{
		while (1)
		{
			over = (s_tmp >= s);
			if (over)
			{
				lambda_tmp += dlambda;
				s_tmp = s_max_m - getSAtLambda(lambda_tmp);
				if (s_tmp < s)
					break;
			}
			else
			{
				lambda_tmp -= dlambda;
				s_tmp = s_max_m - getSAtLambda(lambda_tmp);
				if (s_tmp >= s)
					break;
			}
		}
		if (dlambda < errorTolerance_m / 100.0)
			break;
		dlambda /= 5.0; //through trial and error, this reduces the number of calculations usually (compared with 2, 2.5, 3, 4, 10)
	}

	return lambda_tmp;
}

__host__ __device__ double DipoleB::getBFieldAtS(const double s, const double simtime) const
{// consts: [ ILATDeg, L, L_norm, s_max, ds, errorTolerance ]
	double lambda_deg{ getLambdaAtS(s) };
	double rnorm{ L_norm_m * cospi(lambda_deg / 180.0) * cospi(lambda_deg / 180.0) };

	return -B0 / (rnorm * rnorm * rnorm) * sqrt(1.0 + 3 * sinpi(lambda_deg / 180.0) * sinpi(lambda_deg / 180.0));
}

__host__ __device__ double DipoleB::getGradBAtS(const double s, const double simtime) const
{
	return (getBFieldAtS(s + ds_m, simtime) - getBFieldAtS(s - ds_m, simtime)) / (2 * ds_m);
}

/*__host__ __device__ double DipoleB::getSAtBField(const double B, const double t) const
{
	throw std::logic_error("DipoleB::getSAtBField: function is not ready for use yet.  The author is not confident of its accuracy.");
	double err{ 1.0e-10 };
	double s_guess{
		(B < -25000.0e-9) ? (0.35 * RADIUS_EARTH - 100000.0) / (getBFieldAtS(0.35 * RADIUS_EARTH, t) - getBFieldAtS(100000.0, t)) * B +                           //linefit of 100km - 0.35 RE
			(0.35 * RADIUS_EARTH - (0.35 * RADIUS_EARTH - 100000.0) / (getBFieldAtS(0.35 * RADIUS_EARTH, t) * getBFieldAtS(0.35 * RADIUS_EARTH, t))) :            //calculate b
		(B < -6666.66e-9) ? (1.10 * RADIUS_EARTH - 0.35 * RADIUS_EARTH) / (getBFieldAtS(1.10 * RADIUS_EARTH, t) - getBFieldAtS(0.35 * RADIUS_EARTH, t)) * B +     //linefit of 0.35 RE to 1.10 RE
			(1.10 * RADIUS_EARTH - (1.10 * RADIUS_EARTH - 0.35 * RADIUS_EARTH) / (getBFieldAtS(1.10 * RADIUS_EARTH, t) - getBFieldAtS(0.35 * RADIUS_EARTH, t))) : //calculate b
		(3.00 * RADIUS_EARTH - 1.10 * RADIUS_EARTH) / (getBFieldAtS(3.00 * RADIUS_EARTH, t) - getBFieldAtS(1.10 * RADIUS_EARTH, t)) * B +                         //linefit of 3.00 RE to 1.10 RE
			(3.00 * RADIUS_EARTH - (3.00 * RADIUS_EARTH - 1.10 * RADIUS_EARTH) / (getBFieldAtS(3.00 * RADIUS_EARTH, t) - getBFieldAtS(1.10 * RADIUS_EARTH, t)))   //calculate b
	};
	double delta_s{ 1.0e6 };
	double B_guess{ getBFieldAtS(s_guess, t) };
	bool over{ 0 };

	while (abs((B_guess - B) / B) > err)
	{
		over = (B_guess >= B);
		
		while (1)
		{
			if (over)
			{
				s_guess -= delta_s;
				if (s_guess <= 0.0)
				{
					s_guess += delta_s;
					break;
				}

				B_guess = getBFieldAtS(s_guess, t);

				if (B_guess < B) break;
			}
			else
			{
				s_guess += delta_s;
				B_guess = getBFieldAtS(s_guess, t);
				
				if (B_guess > B) break;
			}
		}
		
		if (delta_s < err) break;
		delta_s /= 5.0;
	}

	return s_guess;
}*/


//DipoleB class member functions
void DipoleB::setupEnvironment()
{// consts: [ ILATDeg, L, L_norm, s_max, ds, errorTolerance ]
	CUDA_API_ERRCHK(cudaMalloc((void **)&this_d, sizeof(BField**)));
	setupEnvironmentGPU_DipoleB <<< 1, 1 >>> (this_d, ILATDegrees_m, errorTolerance_m, ds_m);
	CUDA_KERNEL_ERRCHK_WSYNC();
}

void DipoleB::deleteEnvironment()
{
	deleteEnvironmentGPU_DipoleB <<< 1, 1 >>> (this_d);
	CUDA_KERNEL_ERRCHK_WSYNC();

	CUDA_API_ERRCHK(cudaFree(this_d));
}