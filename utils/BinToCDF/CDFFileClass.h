#ifndef CDFFILE_H
#define CDFFILE_H

#include <iostream>
#include <string>
#include <vector>
#define WIN32 //for cdf
#include "cdf.h"

#define CDFCHKERR() { if (exitStatus_m != CDF_OK) { CDFgetStatusText(exitStatus_m, errTxt_m); std::cout << "Something went wrong: " << exitStatus_m << ":" << errTxt_m << " : " << __FILE__ << ":" << __LINE__ << std::endl; } }

class CDFFileClass //using "Class" just in case there would be any namespace conflicts with the CDF source
{
protected:
	CDFid       cdfid_m;
	std::string filename_m;

	//Attribute and variable characteristics
	std::vector<long>        attrIndicies;
	std::vector<std::string> attrNameStrs;
	std::vector<long>        zVarIndicies;
	std::vector<std::string> zVarNameStrs;

	//Error handling
	CDFstatus   exitStatus_m;
	char        errTxt_m[CDF_STATUSTEXT_LEN + 1];

	long findzVarCDFIndexByName(std::string varName);

public:
	CDFFileClass(std::string filename) : filename_m{ filename }
	{
		if (filename.size() > 1023)
			throw std::invalid_argument ("CDFFileClass::CDFFileClass: filename is too long " + filename);

		char fn[1024];
		for (int chr = 0; chr < filename.size() + 1; chr++)
			fn[chr] = filename.c_str()[chr];
		
		exitStatus_m = CDFcreateCDF(fn, &cdfid_m); //file exists is -2013
	}

	~CDFFileClass()
	{
		CDFcloseCDF(cdfid_m); CDFCHKERR();
	}

	void writeNewZVar(std::string varName, long cdftype, std::vector<int> dimSizes, void* arrayXD);
	void createZVarAttr(long varNum, std::string attrName, long cdftype, void* data);
	void createZVarAttr(std::string varName, std::string attrName, long cdftype, void* data);
};

#endif /* !CDFFILE_H */