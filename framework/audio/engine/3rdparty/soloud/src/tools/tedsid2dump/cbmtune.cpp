#include <string>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "psid.h"
#include "cbmtune.h"

using namespace std;

CbmTune::CbmTune(void) : bufferPtr(0), buffer(0)
{
	playAddress = 0;
	file = 0;
	flen = 0;
	dataIndex = 0;
	initAddress = 0;
	loadAddress = 0;
	dumpLength = 0;
}

CbmTune::~CbmTune(void)
{
	if (buffer) {
		delete buffer;
	}
}

int CbmTune::parse(char *fName)
{
	unsigned int i;
	file = (FILE *) 0;

	if (!fName)
		return 2;

	try {
		file = fopen(fName, "rb");
		fseek(file, 0, SEEK_END);
		flen = ftell(file);
		fseek(file, 0, SEEK_SET);
		buffer = new unsigned char[flen + 1];
		bufferPtr = buffer;
		size_t r = fread(bufferPtr, 1, flen, file);
	} catch(char *str) {
		cerr << "Error opening " << fName << endl;
		cerr << "    exception: " << str << endl;
		return 1;
	}
	// check for magic
	if (strncmp((const char *)(bufferPtr), "CBM8M", 5))
		return 2;
	//
	dataIndex = CBM8M_HDR_STRINGS
		 + (unsigned int) (strlen(getReleaseDate()) + strlen(getAuthor()) + strlen(getName()) + 3);
	if (bufferPtr[CBM8M_HDR_FIELDS] & CBM8M_FIELD_SUBTUNES)
		dataIndex += 1;
	if (bufferPtr[CBM8M_HDR_FIELDS] & CBM8M_FIELD_DEFAULT)
		dataIndex += 1;
	unsigned int stunes = getNrOfSubtunes() + 1;
	unsigned int subTuneFieldOffset = dataIndex;
	dataIndex += stunes;
	// subtune field enum
	for (i = 0; i < stunes; i++) {
		if (bufferPtr[subTuneFieldOffset + i] & 2) {
			dataIndex += 1;
		}
	}
	// read binary config
	unsigned int config = bufferPtr[dataIndex];
	dataIndex += 1;
	// read binary header desc flag
	unsigned int binField = bufferPtr[dataIndex];
	dataIndex += 1;
	// get and store binary dump length
	dumpLength = *((unsigned short *) (bufferPtr + dataIndex));
	dataIndex += 2;
	// optional init address is provided
	if (binField & 1) {
		initAddress = *((unsigned short *) (bufferPtr + dataIndex));
		dataIndex += 2;
	} else {
		initAddress = 0xe2ea;
	}
	// optional play address is provided
	if (binField & 2) {
		playAddress = *((unsigned short *) (bufferPtr + dataIndex));
		dataIndex += 2;
	} else {
		playAddress = 0;
	}
	loadAddress = *((unsigned short *) (bufferPtr + dataIndex));
	dataIndex += 2;
	if (!playAddress) playAddress = 0; //loadAddress;
	return 0;
}

void CbmTune::getPsidHeader(PsidHeader &ph)
{

}
