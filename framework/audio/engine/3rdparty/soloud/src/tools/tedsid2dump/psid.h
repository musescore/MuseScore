#pragma once

// Minimum and maximum header length
const int PSID_MIN_HEADER_LENGTH = 118;		// Version 1
const int PSID_MAX_HEADER_LENGTH = 124;		// Version 2

// Offsets of fields in header (all fields big-endian)
enum {
	PSID_ID = 0,			// 'PSID'
	PSID_VERSION = 4,		// 1 or 2
	PSID_LENGTH = 6,		// Header length
	PSID_START = 8,			// C64 load address
	PSID_INIT = 10,			// C64 init routine address
	PSID_MAIN = 12,			// C64 replay routine address
	PSID_NUMBER = 14,		// Number of subsongs
	PSID_DEFSONG = 16,		// Main subsong number
	PSID_SPEED = 18,		// Speed flags (1 bit/song)
	PSID_NAME = 22,			// Module name (ISO Latin1 character set)
	PSID_AUTHOR = 54,		// Author name (dto.)
	PSID_COPYRIGHT = 86,	// Copyright info (dto.)
	PSID_FLAGS = 118,		// Flags (only in version 2 header)
	PSID_RESERVED = 120
};

struct PsidHeader {
	std::string fileName;
	unsigned int tracks;
	unsigned int initAddress;
	unsigned int replayAddress;
	unsigned int loadAddress;
	unsigned int defaultTune;
	char title[512];
	char author[512];
	char copyright[512];
	char model[32];
	unsigned int type;
	std::string typeName;
	unsigned int version;
	unsigned int current;
	unsigned char *playerCode;
	unsigned int *playerLength;
};

// Read 16-bit quantity from PSID header
inline unsigned short readPsid16(const unsigned char *p, int offset)
{
	return (p[offset] << 8) | p[offset + 1];
}

// Read 32-bit quantity from PSID header
inline unsigned int readPsid32(const unsigned char *p, int offset)
{
	return (p[offset] << 24) | (p[offset + 1] << 16) | (p[offset + 2] << 8) | p[offset + 3];
}

extern bool psidChangeTrack(int direction);
extern void printPsidInfo(PsidHeader &psidHdr_);
extern PsidHeader &getPsidHeader();
extern void getPsidProperties(PsidHeader &psidHdr_, char *text);
