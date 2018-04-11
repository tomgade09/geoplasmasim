#include "FileIO\fileIO.h"

namespace fileIO
{
	void readDblBin(std::vector<double>& arrayToReadInto, std::string filename)
	{
		std::ifstream binFile{ filename, std::ios::binary };
		if (!binFile.is_open())
			throw std::invalid_argument("fileIO::readDblBin: could not open file " + filename + " for reading");

		binFile.seekg(0, binFile.end);
		int length{ (int)(binFile.tellg()) / 8 };
		binFile.seekg(0, binFile.beg);
		binFile.close();
		std::cout << "fileIO::readDblBin: (unknown size): num elements determined from disk: " << length << std::endl;

		arrayToReadInto.resize(length);

		readDblBin(arrayToReadInto, filename, (long)length);
	}

	void readDblBin(std::vector<double>& arrayToReadInto, std::string filename, long numOfDblsToRead)
	{
		std::ifstream binFile{ filename, std::ios::binary };
		if (!binFile.is_open())
			throw std::invalid_argument("fileIO::readDblBin: could not open file " + filename + " for reading");
		if (arrayToReadInto.size() < numOfDblsToRead)
		{
			binFile.close();
			throw std::invalid_argument("fileIO::readDblBin: std::vector is not big enough to contain the data being read from file " + filename);
		}
		
		binFile.seekg(0, binFile.end);
		int length{ (int)(binFile.tellg()) };
		binFile.seekg(0, binFile.beg);

		if (length < numOfDblsToRead * 8)
		{
			binFile.close();
			throw std::invalid_argument("fileIO::readDblBin: filesize of \"" + filename + "\" is smaller than specified number of doubles to read");
		}
		if (length > numOfDblsToRead * 8)
			std::cerr << "fileIO::readDblBin: warning: size of data read is less than the size of all data in file " << filename << ": continuing" << std::endl;

		binFile.read(reinterpret_cast<char*>(arrayToReadInto.data()), std::streamsize(numOfDblsToRead * sizeof(double)));
		binFile.close();
	}

	void read2DCSV(std::vector<std::vector<double>>& array2DToReadInto, std::string filename, int numofentries, int numofcols, const char delim)
	{
		std::ifstream csv{ filename };
		if (!csv.is_open())
			throw std::invalid_argument("fileIO::read2DCSV: could not open file " + filename + " for reading");
		if (array2DToReadInto.size() < numofcols)
			throw std::invalid_argument("fileIO::read2DCSV: std::vector outer vector is not big enough to contain the data being read from file " + filename);
		if (array2DToReadInto.size() > numofcols)
			std::cerr << "fileIO::read2DCSV: std::vector outer vector is bigger than numofcols, some data in the vector will remain unmodified" << std::endl;
		for (int col = 0; col < array2DToReadInto.size(); col++)
		{
			if (array2DToReadInto.at(col).size() < numofentries)
				throw std::invalid_argument("fileIO::read2DCSV: std::vector inner vector is not big enough to contain the data being read from file " + filename);
			if (array2DToReadInto.at(col).size() > numofentries)
				std::cerr << "fileIO::read2DCSV: std::vector inner vector is bigger than numofentries, some data in the vector will remain unmodified" << std::endl;
		}

		try
		{
			for (int iii = 0; iii < numofentries; iii++)
			{
				std::string in;
				std::getline(csv, in);

				std::stringstream in_ss(in);

				for (int jjj = 0; jjj < numofcols; jjj++)
				{
					std::string val;
					std::getline(in_ss, val, delim);
					std::stringstream convert(val);
					convert >> array2DToReadInto.at(jjj).at(iii);
				}
			}
		}
		catch(...)
		{
			csv.close();
			throw;
		}

		csv.close();
	}

	void readTxtFile(std::string& readInto, std::string filename)
	{
		std::ifstream txt(filename);
		if (!txt.is_open())
		{
			txt.close();
			throw std::invalid_argument("fileIO::readTxtFile: could not open file " + filename + " for reading " + std::to_string(txt.is_open()));
		}

		std::stringstream buf;
		buf << txt.rdbuf();
		txt.close();

		readInto = buf.str();
	}


	//write functions
	void writeDblBin(const std::vector<double>& dataarray, std::string filename, long numelements, bool overwrite)//overwrite defaults to true
	{
		std::ofstream binfile{ filename, std::ios::binary | (overwrite ? (std::ios::trunc) : (std::ios::app)) };
		if (!binfile.is_open())
			throw std::invalid_argument("fileIO::writeDblBin: could not open file " + filename + " for writing");
		if (dataarray.size() < numelements)
		{
			binfile.close();
			throw std::invalid_argument("fileIO::writeDblBin: size of data vector is less than the number of doubles requested from it for filename " + filename);
		}

		binfile.write(reinterpret_cast<const char*>(dataarray.data()), std::streamsize(numelements * sizeof(double)));
		binfile.close();
	}

	void write2DCSV(const std::vector<std::vector<double>>& dataarray, std::string filename, int numofentries, int numofcols, const char delim, bool overwrite, int precision)//overwrite defaults to true, precision to 20
	{
		std::ofstream csv(filename, overwrite ? (std::ios::trunc) : (std::ios::app));
		if (!csv.is_open())
			throw std::invalid_argument("fileIO::write2DCSV: could not open file " + filename + " for writing");
		if (dataarray.size() < numofcols)
		{
			csv.close();
			throw std::invalid_argument("fileIO::write2DCSV: size of data vector is less than the doubles requested from it for filename " + filename);
		}

		for (int iii = 0; iii < numofentries; iii++)
		{
			for (int jjj = 0; jjj < numofcols; jjj++)
				csv << std::setprecision(precision) << dataarray.at(jjj).at(iii) << delim;
			
			csv << "\n";
		}

		csv.close();
		
		return;
	}

	void writeTxtFile(std::string textToWrite, std::string filename, bool overwrite)//overwrite defaults to false
	{
		std::ofstream txt(filename, overwrite ? (std::ios::trunc) : (std::ios::app));
		if (!txt.is_open())
		{
			txt.close();
			throw std::invalid_argument("fileIO::writeTxtFile: could not open file " + filename + " for writing " + std::to_string(txt.is_open()));
		}

		txt << textToWrite;

		txt.close();
	}

	void writeAttrsToFiles(std::vector<double> attrs, std::vector<std::string> charNames, std::string className, std::string saveFolder)
	{
		FILE_RDWR_EXCEP_CHECK(writeDblBin(attrs, saveFolder + className + ".bin", (int)attrs.size()));

		std::string charNamesStr{ "" };
		for (int charNm = 0; charNm < charNames.size(); charNm++)
		{
			charNamesStr += charNames.at(charNm);
			if (charNm != charNames.size() - 1)
				charNamesStr += ",";
		}

		FILE_RDWR_EXCEP_CHECK(writeTxtFile(charNamesStr, saveFolder + className + ".txt", true));
	}
}