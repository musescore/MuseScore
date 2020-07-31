#pragma once

#include <cstdlib>
#include <cstring>

enum Cbm8header {
	CBM8M_HDR_MAGIC = 0,
	CBM8M_HDR_VERSION = 5,
	CBM8M_HDR_PLATFORM,
	CBM8M_HDR_HWFLAGS,
	CBM8M_HDR_FIELDS,
	CBM8M_HDR_STRINGS
};

enum Cbm8fields {
	CBM8M_FIELD_SUBTUNES = 1 << 0,
	CBM8M_FIELD_DEFAULT = 1 << 1,
	CBM8M_FIELD_BINDUMPS = 1 << 2,
	CBM8M_FIELD_COMMENT = 1 << 3
};

enum Cbm8binDumpHdr {
	CBM8M_BIN_HW_FLAGS = 0,
	CBM8M_BIN_FIELDS,
	CBM8M_BIN_LENGTH,
	CBM8M_BIN_INIT_ADDRESS,
	CBM8M_BIN_INIT_ADDRESS_HI,
	CBM8M_BIN_PLAY_ADDRESS,
	CBM8M_BIN_PLAY_ADDRESS_HI,
	CBM8M_BIN_AREA_UNUSED,
	CBM8M_BIN_AREA_UNUSED_HI,
	CBM8M_BIN_DATA
};

class CbmTune
{
public:
	CbmTune(void);
	virtual ~CbmTune(void);
	int parse(char *fName);
	void getPsidHeader(PsidHeader &ph);
	virtual char *getName() {
		return (char *) bufferPtr + CBM8M_HDR_STRINGS;
	}
	virtual char *getAuthor() {
		return (char *) bufferPtr + CBM8M_HDR_STRINGS + strlen(getName()) + 1;
	}
	virtual char *getReleaseDate() {
		return (char *) bufferPtr + CBM8M_HDR_STRINGS
			+ strlen(getAuthor()) + strlen(getName()) + 2;
	}
	virtual unsigned int getNrOfSubtunes() {
		if (bufferPtr[CBM8M_HDR_FIELDS] & CBM8M_FIELD_SUBTUNES)
			return bufferPtr[CBM8M_HDR_STRINGS
				+ strlen(getReleaseDate()) + strlen(getAuthor()) + strlen(getName()) + 3];
		else
			return 0;
	}
	virtual unsigned int getDefaultSubtune() {
		if (bufferPtr[CBM8M_HDR_FIELDS] & CBM8M_FIELD_DEFAULT)
			return bufferPtr[CBM8M_HDR_FIELDS
				+ 1
				+ strlen(getReleaseDate()) + strlen(getAuthor()) + strlen(getName()) + 3];
		else
			return 0;
	}
	//virtual char *getComment() {
	//	if (bufferPtr[CBM8M_HDR_FIELDS] & CBM8M_FIELD_COMMENT)
	//		return (char *) bufferPtr + CBM8M_HDR_FIELDS
	//			+ getNrOfSubtunes()
	//			+ strlen(getReleaseDate()) + strlen(getAuthor()) + strlen(getName()) + 3];
	//	else
	//		return "";
	//}
	virtual unsigned short getInitAddress(unsigned int nr) {
		//return *((unsigned short *) (bufferPtr + dataIndex + 4));
		return initAddress;
	}
	virtual unsigned short getPlayAddress(unsigned int nr) {
		//return *((unsigned short *) (bufferPtr + dataIndex + 2));
		return playAddress;
	}
	virtual unsigned short getDumpLength(unsigned int nr) {
		return dumpLength;
	}
	virtual unsigned short getLoadAddress(unsigned int nr) {
		//return *((unsigned short *) (bufferPtr + dataIndex + 4));
		return loadAddress;
	}
	virtual unsigned char *getBinaryData(unsigned int nr) {
		//return (bufferPtr + dataIndex + 6);
		return (bufferPtr + dataIndex );
	}

protected:
	FILE *file;
	size_t flen;
	unsigned char *buffer;
	unsigned char *bufferPtr;
	unsigned int dataIndex;
	unsigned int initAddress;
	unsigned int playAddress;
	unsigned int loadAddress;
	unsigned int dumpLength;
};
