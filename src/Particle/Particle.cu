#include <ctime>

//CUDA includes
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda_profiler_api.h"
#include "curand_kernel.h"

#include "ErrorHandling/cudaErrorCheck.h"
#include "ErrorHandling/simExceptionMacros.h"

#include "Particle/Particle.h"
#include "physicalconstants.h"
#include "utils/loopmacros.h"
#include "utils/fileIO.h"

using utils::fileIO::readDblBin;
using utils::fileIO::writeDblBin;

Particle::Particle(std::string name, std::vector<std::string> attributeNames, double mass, double charge, long numParts) :
	name_m{ name }, attributeNames_m{ attributeNames }, mass_m{ mass }, charge_m{ charge }, numberOfParticles_m{ numParts }
{
	origData_m = std::vector<std::vector<double>>(attributeNames.size(), std::vector<double>(numParts));
	currData_m = std::vector<std::vector<double>>(attributeNames.size(), std::vector<double>(numParts));

	initializeGPU();
}

Particle::~Particle()
{
	freeGPUMemory();
}

//Device code (kernels)
__global__ void setup2DArray(double* array1D, double** array2D, int cols, int entries)
{//run once on only one thread
	if (blockIdx.x * blockDim.x + threadIdx.x != 0)
		return;

	for (int iii = 0; iii < cols; iii++)
		array2D[iii] = &array1D[iii * entries];
}

//curand code for generating random particles
__global__ void initCurand(curandStateMRG32k3a* state, long long seed)
{
	unsigned int thdInd{ blockIdx.x * blockDim.x + threadIdx.x };
	curand_init(seed, thdInd, 0, &state[thdInd]);
}

__global__ void randomGenerator(double** currData, double vmean, double vsigma, double mass, int offset, curandStateMRG32k3a* rndStates, int numRngStates)
{//double data[]{ dt_m, simMin_m, simMax_m, ionT_m, magT_m, vmean_m }
	unsigned int thdInd{ blockIdx.x * blockDim.x + threadIdx.x };

	double2 v_norm; //v_norm.x = v_para; v_norm.y = v_perp
	v_norm = curand_normal2_double(&rndStates[thdInd % numRngStates]);
	v_norm.x = v_norm.x * vsigma + vmean; //normal dist -> maxwellian
	v_norm.y = v_norm.y * vsigma + vmean; //normal dist -> maxwellian

	currData[0][thdInd + offset] = abs(v_norm.x);
	currData[1][thdInd + offset] = v_norm.y;
	//s is not set!!!
}


//Host code
//the problem with below is that it's based on a certain attribute structure - this needs to be removed from the class and put somewhere else
void Particle::generateRandomParticles(const std::vector<double>& s, int startInd, int length, double vmean, double kBT_eV, double mass)
{//not tested - give a good test
	if (length != s.size()) { throw std::invalid_argument("Particle::generateRandomParticles: s.size is not equal to number of particles to generate - s.size: " + std::to_string(s.size()) + " length: " + std::to_string(length)); }
	if ((startInd + length) > numberOfParticles_m) { throw std::invalid_argument("Particle::generateRandomParticles: length + startInd is bigger than the size of the data array - length + startInd: " + std::to_string(length + startInd) + " num Particles: " + std::to_string(numberOfParticles_m)); }
	if (length > 2 * 65536 * 256) { std::cout << "Particle::generateRandomParticles: Warning: more than 256 threads per random number generator state - this exceeds the thread/state recommendations by CUDA's documentation"; }

	int blocksize{ 0 };
	if (length % 256 == 0) //maybe add log entry at this point
		blocksize = 256;
	else if (length % 128 == 0)
		blocksize = 128;
	else if (length % 64 == 0)
		blocksize = 64;
	else if (length % 16 == 0)
		blocksize = 16;
	else if (length % 4 == 0)
		blocksize = 4;
	else if (length % 2 == 0)
		blocksize = 2;
	else
		blocksize = 1;

	int numRNGstates{ (length > 65536 * 256) ? 65536 * 2 : 65536 };
	
	curandStateMRG32k3a* curandRNGStates_d; //sizeof is 72 bytes
	CUDA_API_ERRCHK(cudaMalloc((void **)&curandRNGStates_d, numRNGstates * sizeof(curandStateMRG32k3a))); //Array of 65536 random number generator states
	
	//Prepare curand states for random number generation
	long long seed = std::time(nullptr);
	initCurand <<< numRNGstates / 128, 128 >>> (curandRNGStates_d, seed);
	CUDA_KERNEL_ERRCHK();

	//generate stuff
	double vsigma{ sqrt(kBT_eV * JOULE_PER_EV / (2 * mass)) }; //think I did this right since both v's are generated by a maxwellian - 2 might need to be outside sqrt
	randomGenerator <<< length / blocksize, blocksize >>> (currData2D_d, vmean, vsigma, mass, startInd, curandRNGStates_d, numRNGstates);
	CUDA_KERNEL_ERRCHK();

	CUDA_API_ERRCHK(cudaFree(curandRNGStates_d));

	// Doing something stupid, but it will work for now
	// Generating particles on GPU, copying to host, erasing GPU data
	// Later copyDataToGPU will be called and copy it back
	// This is done because copyDataToGPU is called no matter what
	// And the data on device here will be overwritten with zeroes

	//test this especially
	LOOP_OVER_1D_ARRAY(2, CUDA_API_ERRCHK(cudaMemcpy(currData1D_d + iii * numberOfParticles_m + startInd, origData_m.at(iii).data() + startInd, length * sizeof(double), cudaMemcpyDeviceToHost)));
	std::copy(s.begin(), s.end(), origData_m.at(2).begin() + startInd);

	CUDA_API_ERRCHK(cudaMemset(currData1D_d, 0, sizeof(double) * (int)attributeNames_m.size() * numberOfParticles_m));

	initDataLoaded_m = true;
}

void Particle::initializeGPU()
{
	size_t memSize{ numberOfParticles_m * (getNumberOfAttributes()) * sizeof(double) };
	CUDA_API_ERRCHK(cudaMalloc((void **)&origData1D_d, memSize));
	CUDA_API_ERRCHK(cudaMalloc((void **)&currData1D_d, memSize));
	CUDA_API_ERRCHK(cudaMalloc((void **)&origData2D_d, getNumberOfAttributes() * sizeof(double*)));
	CUDA_API_ERRCHK(cudaMalloc((void **)&currData2D_d, getNumberOfAttributes() * sizeof(double*)));

	CUDA_API_ERRCHK(cudaMemset(origData1D_d, 0, memSize));
	CUDA_API_ERRCHK(cudaMemset(currData1D_d, 0, memSize));

	setup2DArray <<< 1, 1 >>> (origData1D_d, origData2D_d, getNumberOfAttributes(), numberOfParticles_m);
	setup2DArray <<< 1, 1 >>> (currData1D_d, currData2D_d, getNumberOfAttributes(), numberOfParticles_m);
	CUDA_KERNEL_ERRCHK_WSYNC();

	dataOnGPU_m = true;
}

void Particle::copyDataToGPU()
{
	if (!dataOnGPU_m)
		throw std::logic_error("Particle::copyDataToGPU: GPU memory has not been initialized yet for particle " + name_m);
	if (!initDataLoaded_m)
		throw std::runtime_error("Particle::copyDataToGPU: data not loaded from disk with Particle::loadDataFromDisk or generated with Particle::generateRandomParticles " + name_m);

	size_t memSize{ numberOfParticles_m * sizeof(double) }; //load data to origData, move to currData on GPU
	LOOP_OVER_1D_ARRAY(getNumberOfAttributes(), CUDA_API_ERRCHK(cudaMemcpy(currData1D_d + numberOfParticles_m * iii, origData_m.at(iii).data(), memSize, cudaMemcpyHostToDevice)));
}

void Particle::copyDataToHost()
{
	if (!dataOnGPU_m)
		throw std::logic_error("Particle::copyDataToHost: GPU memory has not been initialized yet for particle " + name_m);

	size_t memSize{ numberOfParticles_m * sizeof(double) };
	//LOOP_OVER_1D_ARRAY(getNumberOfAttributes(), CUDA_API_ERRCHK(cudaMemcpy(origData_m.at(iii).data(), origData1D_d + numberOfParticles_m * iii, memSize, cudaMemcpyDeviceToHost)));
	LOOP_OVER_1D_ARRAY(getNumberOfAttributes(), CUDA_API_ERRCHK(cudaMemcpy(currData_m.at(iii).data(), currData1D_d + numberOfParticles_m * iii, memSize, cudaMemcpyDeviceToHost))); //coming back curr(GPU) -> curr(host)
}

void Particle::freeGPUMemory()
{
	if (!dataOnGPU_m) { return; }

	CUDA_API_ERRCHK(cudaFree(origData1D_d));
	CUDA_API_ERRCHK(cudaFree(currData1D_d));
	CUDA_API_ERRCHK(cudaFree(origData2D_d));
	CUDA_API_ERRCHK(cudaFree(currData2D_d));

	origData1D_d = nullptr;
	currData1D_d = nullptr;
	origData2D_d = nullptr;
	currData2D_d = nullptr;

	dataOnGPU_m = false;
}

void Particle::clearGPUMemory()
{
	if (origData1D_d && currData1D_d)
	{
		CUDA_API_ERRCHK(cudaMemset(origData1D_d, 0, sizeof(double) * (int)attributeNames_m.size() * numberOfParticles_m));
		CUDA_API_ERRCHK(cudaMemset(currData1D_d, 0, sizeof(double) * (int)attributeNames_m.size() * numberOfParticles_m));
	}
}

//these below could probably be removed to a cpp file

//need a custom solution for this...
//file read/write exception checking (probably should mostly wrap fileIO functions)
#define FILE_RDWR_EXCEP_CHECK(x) \
	try{ x; } \
	catch(const std::invalid_argument& a) { std::cerr << __FILE__ << ":" << __LINE__ << " : " << "Invalid argument error: " << a.what() << ": continuing without loading file" << std::endl; std::cout << "FileIO exception: check log file for details" << std::endl; } \
	catch(...)                            { throw; }


void Particle::loadDataFromDisk(std::string folder, bool orig)
{
	for (int attrs = 0; attrs < attributeNames_m.size(); attrs++)
		FILE_RDWR_EXCEP_CHECK(readDblBin((orig ? origData_m.at(attrs) : currData_m.at(attrs)), folder + "/" + name_m + "_" + attributeNames_m.at(attrs) + ".bin", numberOfParticles_m));

	initDataLoaded_m = true;
}

void Particle::saveDataToDisk(std::string folder, bool orig) const
{
	for (int attrs = 0; attrs < attributeNames_m.size(); attrs++)
		FILE_RDWR_EXCEP_CHECK(writeDblBin((orig ? origData_m.at(attrs) : currData_m.at(attrs)), folder + "/" + name_m + "_" + attributeNames_m.at(attrs) + ".bin", numberOfParticles_m));
}

int Particle::getAttrIndByName(std::string searchName) const
{
	for (int name = 0; name < attributeNames_m.size(); name++)
		if (searchName == attributeNames_m.at(name))
			return name;

	throw std::invalid_argument("Particle::getDimensionIndByName: specified name is not present in name array: " + searchName);
}

std::string Particle::getAttrNameByInd(int searchIndx) const
{
	if (!(searchIndx <= (attributeNames_m.size() - 1) && (searchIndx >= 0)))
		throw std::invalid_argument("Particle::getDimensionNameByInd: specified index is invalid: " + std::to_string(searchIndx));

	return attributeNames_m.at(searchIndx);
}