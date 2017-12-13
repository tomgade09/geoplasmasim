#include "StandaloneTools\binaryfiletools.h"

void saveParticleAttributeToDisk(double* arrayToSave, int length, const char* foldername, const char* name)
{
	std::string fn{ foldername };
	fn = fn + name;
	fileIO::writeDblBin(fn.c_str(), arrayToSave, length);
}

void loadFileIntoParticleAttribute(double* arrayToLoadInto, int length, const char* foldername, const char* name)
{
	delete[] arrayToLoadInto;
	std::string fn{ foldername };
	fn = fn + name;
	arrayToLoadInto = fileIO::readDblBin(fn.c_str(), length);
}

void stringPadder(std::string& in, int totalStrLen, int indEraseFrom)
{
	if (totalStrLen <= 0 || indEraseFrom <= 0)
		return;

	int txtlen = in.length();

	if ((totalStrLen - txtlen) > 0)
	{
		for (int iii = 0; iii < (totalStrLen - txtlen); iii++)
			in += ' ';
	}
	else
		in.erase(indEraseFrom, txtlen - totalStrLen);
}