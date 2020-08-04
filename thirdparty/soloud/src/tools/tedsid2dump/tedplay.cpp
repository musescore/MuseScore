#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include "psid.h"
#include "CbmTune.h"
#include "Tedmem.h"
#include "Cpu.h"
#include "Sid.h"

#define MAX_BUFFER_SIZE 0x10000

static unsigned char inputBuffer[MAX_BUFFER_SIZE];
static const unsigned int playerStartAddress = 0xfe00;
static unsigned char psidPlayer[] = {
	0xA9, 0x00 // song nr (offset 2)
	,0x20 ,
	0x33 , 0xfe, // song init (offset 4)
	0x78 , 0xA9 , 0x00 , 0x8D ,
	0x0B , 0xFF , 0xA9 , 0x38 ,
	0xCD , 0x1D , 0xFF , 0xD0 ,
	0xFB , 0xCE , 0x19 , 0xFF ,
	0x20 , 0x33 , 0xfe  // song play (offset 23)
	, 0xEE , 0x19 , 0xFF , 0x4C , 0x0B , 0xfe, 
	0x00 , 0x60 // fake RTS at $FE33
};

static unsigned char rsidPlayer[] = {
	0xA9, 0x00, // song nr (offset 2)
	0x20,
	0xea, 0xe2, // song init (offset 3)
	0x18, 0x90, 0xfe // song init (offset 5)
};

static unsigned char prgPlayer[] = {
	0x20, 0xbe, 0x8b,
	0x4c, 0xdc, 0x8b
};

static TED *ted = NULL;
static CPU *cpu;
static PsidHeader psidHdr;
static int playState = 1;

void process(int ticks)
{
	ted->ted_process(ticks);
}

PsidHeader &getPsidHeader()
{
	return psidHdr;
}

unsigned int parsePsid(unsigned char *buf, PsidHeader &psidHdr_)
{
	char *buffer = (char *) buf;

	if (buf) {
		memset(psidHdr_.title, 0, sizeof(psidHdr_.title));
		strncpy(psidHdr_.title, buffer + PSID_NAME, 32);
		psidHdr_.title[PSID_NAME + 32] = 0;
		strncpy(psidHdr_.author, buffer + PSID_AUTHOR, 32);
		psidHdr_.author[PSID_AUTHOR + 32] = 0;
		strncpy(psidHdr_.copyright, buffer + PSID_COPYRIGHT, 32);
		psidHdr_.copyright[PSID_COPYRIGHT + 32] = 0;
	} else {
		memset(psidHdr_.title, 0, sizeof(psidHdr_.title));
		memset(psidHdr_.author, 0, 32);
		memset(psidHdr_.copyright, 0, 32);
		memset(psidHdr_.copyright, 0, 32);
	}
	return 0;
}

void getPsidProperties(PsidHeader &psidHdr_, char *os)
{
	char temp[1024];

	if (!psidHdr_.tracks) {
		strcat(os, "");
		return;
	}
	std::string mType;
	switch (psidHdr_.type) {
		default:
			mType = "Other";
			break;
		case 0:
			mType = "PSID";
			break;
		case 1:
			mType = "RSID";
			break;
		case 2:
			mType = "CBM8M";
			break;
	}
	sprintf(os,   "Type:         %s\r\n", mType.c_str());
	sprintf(temp, "Chip:         %s\r\n", psidHdr_.model);
	strcat(os, temp);
	sprintf(temp, "Module:       %s\r\n", psidHdr_.title);
	strcat(os, temp);
	sprintf(temp, "Author:       %s\r\n", psidHdr_.author);
	strcat(os, temp);
	sprintf(temp, "Released:     %s\r\n", psidHdr_.copyright);
	strcat(os, temp);
	sprintf(temp, "Total tunes:  %u\r\n", psidHdr_.tracks);
	strcat(os, temp);
	sprintf(temp, "Default tune: %u\r\n", psidHdr_.defaultTune);
	strcat(os, temp);
	sprintf(temp, "Init        : $%04X\r\n", psidHdr_.initAddress);
	strcat(os, temp);
	sprintf(temp, "Play address: $%04X\r\n", psidHdr_.replayAddress);
	strcat(os, temp);
}

void printPsidInfo(PsidHeader &psidHdr_)
{
	char output[1024];
	getPsidProperties(psidHdr_, output);
	std::cout << std::string(output) << std::endl;
}

unsigned int readFile(char *fName, unsigned char **bufferPtr, size_t *bLen)
{
	std::FILE *file = (std::FILE *) 0;

	if (!fName)
		return 2;

	try {
		size_t flen;
		file = std::fopen(fName, "rb");
		if (!file)
		{
			std::cerr << "File not found " << fName << std::endl;
			return 3;
		}
		std::fseek(file, 0, SEEK_END);
		flen = std::ftell(file);
		if (flen > MAX_BUFFER_SIZE) flen = MAX_BUFFER_SIZE;
		std::fseek(file, 0, SEEK_SET);
		*bufferPtr = inputBuffer;
		size_t r = std::fread(*bufferPtr, flen, 1, file);
		*bLen = flen;
		fclose(file);
	} catch(char *str) {
		std::cerr << "Error opening " << fName << std::endl;
		std::cerr << "    exception: " << str << std::endl;
		return 1;
	}
	return 0;
}

bool psidChangeTrack(int direction)
{
	if (direction > 0) {
		if (psidHdr.tracks > psidHdr.current) {
			psidPlayer[1] += direction;
			rsidPlayer[1] += direction;
			psidHdr.current += direction;
		} else {
			std::cerr << "No more tracks." << std::endl;
			return false;
		}
	} else {
		if (1 < psidHdr.current) {
			psidPlayer[1] += direction;
			rsidPlayer[1] += direction;
			psidHdr.current += direction;
		} else {
			std::cerr << "No more tracks." << std::endl;
			return false;
		}
	}
	if (psidHdr.type == 1 || (psidHdr.type == 2 && !psidHdr.replayAddress)) {
		ted->writeProtectedPlayerMemory(playerStartAddress, rsidPlayer, sizeof(rsidPlayer));
	} else {
		ted->writeProtectedPlayerMemory(playerStartAddress, psidPlayer, sizeof(psidPlayer));
	}
	cpu->setPC(playerStartAddress);
	return true;
}

void machineReset()
{
	ted->forcedReset();
	cpu->Reset();
}

TED *machineInit()
{
	ted = new TED;
	cpu = new CPU(ted, ted->Ram + 0xff09, ted->Ram + 0x100);
	ted->cpuptr = cpu;
	ted->loadroms();
	machineReset();
	return ted;
}

void tedPlaySetFilterOrder(unsigned int filterOrder)
{
	if (ted) {
		ted->setFilterOrder(filterOrder);
	}
}

void dumpMem(std::string name)
{
	FILE *fp = fopen(name.c_str(), "wb");
	if (!fp)
		return;
	for(unsigned int i = 0; i <= 0xffff; i++) {
		fputc(ted->Read(i), fp);
	}
	fclose(fp);
}

void machineDoSomeFrames(unsigned int count)
{
}

void machineShutDown()
{
	if(ted) {
		delete ted;
		ted = 0;
	}
	if (cpu) {
		delete cpu;
		cpu = 0;
	}
}

void tedplayPause()
{
}

void tedplayPlay()
{
}

void tedplayStop()
{
}

unsigned int tedplayGetSecondsPlayed()
{
	unsigned int sec = ted->getTimeSinceLastReset();
	return sec;
}

void tedPlayResetCycleCounter()
{
	ted->resetCycleCounter();
}

int tedPlayGetState()
{
	return playState;
}

unsigned int tedPlayGetWaveform(unsigned int channel)
{
	if (ted) {
		return ted->getWaveForm(channel);
	}
	return 0;
}

void tedPlaySetWaveform(unsigned int channel, unsigned int wave)
{
	bool wasPlaying = (playState == 1);
	
	if (wasPlaying)
		tedplayPause();
	if (ted) {
		ted->selectWaveForm(channel, wave);
	}
	if (wasPlaying)
		tedplayPlay();
}

void tedplayClose()
{
	machineShutDown();
}

void tedPlayGetInfo(void *file, PsidHeader &hdr)
{
	unsigned char buf[256];
	FILE *fp = reinterpret_cast<FILE *>(file);

	memset(hdr.title, 0, sizeof(hdr.title));
	strcpy(hdr.title, "Unknown");
	memset(hdr.author, 0, sizeof(hdr.author));
	strcpy(hdr.author, "Unknown");
	memset(hdr.copyright, 0, sizeof(hdr.copyright));
	strcpy(hdr.copyright, "Unknown");

	if (fread(buf, 1, 256, fp) >= 64) {
		if (!strncmp((const char *) buf + 1, "SID", 3)) {
			parsePsid(buf, hdr);
			hdr.loadAddress = buf[PSID_START + 1] + (buf[PSID_START] << 8);
			if (!hdr.loadAddress)
				hdr.loadAddress = buf[PSID_MAX_HEADER_LENGTH] + (buf[PSID_MAX_HEADER_LENGTH + 1] << 8);
			if (buf[0] == 'P') {
				hdr.typeName = "PSID";
			} else {
				hdr.typeName = "RSID";
			}
		} else if (!strncmp((const char *) buf, "CBM8M", 5)) {
			CbmTune tune;
			//tune.parse(
			hdr.typeName = "CBM8M";
		} else {
			hdr.typeName = "PRG";
			hdr.loadAddress = buf[0] + (buf[1] << 8);
		}
	}
}

int selected_model = 1;

int tedplayMain(char *fileName, int model)
{
	size_t bufLength;
	unsigned char *buf = 0;

	if (!readFile(fileName, &buf, &bufLength)) {

		tedplayPause();
		machineReset();
		tedplayPlay();
		
		int i;
		for (i = 0; i < 100; i++)
		process(8192); // reset sequence

		tedplayStop();

		psidHdr.fileName = fileName;

		if (!strncmp((const char *) buf + 1, "SID", 3)) {
			psidHdr.loadAddress = buf[PSID_START + 1] + (buf[PSID_START] << 8);
			unsigned int corr = 0;
			// zero load address means PRG module
			if (!psidHdr.loadAddress) {
				psidHdr.loadAddress = buf[PSID_MAX_HEADER_LENGTH] + (buf[PSID_MAX_HEADER_LENGTH + 1] << 8);
				corr = 2;
			}
			unsigned short initAddr = buf[PSID_INIT + 1] + (buf[PSID_INIT] << 8);
			unsigned short replayAddr = buf[PSID_MAIN + 1] + (buf[PSID_MAIN] << 8);
			// zero init address means equal to load address
			psidHdr.initAddress = initAddr ? initAddr : psidHdr.loadAddress;
			// 0 means an IRQ handler will be installed in the init routine
			// (most likely nothing happens)
			psidHdr.replayAddress = replayAddr ? replayAddr : 0xfe33;
			psidHdr.defaultTune = readPsid16(buf, PSID_DEFSONG);
			psidHdr.current = psidHdr.defaultTune;
			psidHdr.version = readPsid16(buf, PSID_VERSION);
			psidHdr.tracks = readPsid16(buf, PSID_NUMBER);
			parsePsid(buf, psidHdr);

			// setup the SID card
			SIDsound *sid = ted->getSidCard();
			if (sid) {
				// Disable the ROM & IRQ's, so almost the entire memory is RAM
				// more SID's are likely to play
				ted->cpuptr->setST(ted->cpuptr->getST() | 4);
				ted->ChangeMemBankSetup(true);
				// v2 SID files have flags
				if (1) {//psidHdr.version == 2 && (readPsid16(buf, PSID_FLAGS) & 0x20)) {
					sid->setModel(SID8580);
					selected_model = SID8580;
					strcpy(psidHdr.model, "SID8580");
				} else {
					sid->setModel(SID6581);
					selected_model = SID6581;
					strcpy(psidHdr.model, "SID6581");
				}
			} else {
				strcpy(psidHdr.model, "TED8360?");
			}
			if (buf[0] == 'P') { // PSID
				psidHdr.type = 0;
				psidPlayer[1] = psidHdr.defaultTune - 1;
				psidPlayer[3] = psidHdr.initAddress & 0xff;
				psidPlayer[4] = psidHdr.initAddress >> 8;
				psidPlayer[22] = psidHdr.replayAddress & 0xff;
				psidPlayer[23] = psidHdr.replayAddress >> 8;
				ted->writeProtectedPlayerMemory(playerStartAddress, psidPlayer, sizeof(psidPlayer));
			} else if (buf[0] == 'R') { // RSID
				psidHdr.type = 1;
				rsidPlayer[1] = psidHdr.defaultTune - 1;
				rsidPlayer[3] = psidHdr.initAddress & 0xff;
				rsidPlayer[4] = psidHdr.initAddress >> 8;
				if (replayAddr != 0) {
					rsidPlayer[5] = 0x4C; // JMP
					rsidPlayer[6] = psidHdr.replayAddress & 0xff;
					rsidPlayer[7] = psidHdr.replayAddress >> 8;
				} else {
					rsidPlayer[5] = 0x18; // CLC
					rsidPlayer[6] = 0x90; // BCC *-2
					rsidPlayer[7] = 0xfe;
				}
				ted->writeProtectedPlayerMemory(playerStartAddress, rsidPlayer, sizeof(rsidPlayer));
			}
			ted->injectCodeToRAM(psidHdr.loadAddress, buf + PSID_MAX_HEADER_LENGTH + corr,
				bufLength - PSID_MAX_HEADER_LENGTH - corr);

		} else if (!strncmp((const char *) buf, "CBM8M", 5)) {
			CbmTune tune;
			tune.parse(fileName);
			psidHdr.type = 2;
			strcpy(psidHdr.title, tune.getName());
			strcpy(psidHdr.author, tune.getAuthor());
			strcpy(psidHdr.copyright, tune.getReleaseDate());
			unsigned int dataLen = tune.getDumpLength(0);
			psidHdr.loadAddress = tune.getLoadAddress(0);
			unsigned short initAddr = tune.getInitAddress(0);
			unsigned short replayAddr = tune.getPlayAddress(0);
			unsigned char *playerData;
			unsigned int playerLength;
			psidHdr.version = 1;
			psidHdr.tracks = tune.getNrOfSubtunes() + 1;
			psidHdr.defaultTune = tune.getDefaultSubtune() + 1;
			psidHdr.current = psidHdr.defaultTune;
			psidHdr.initAddress = initAddr;
			psidHdr.replayAddress = replayAddr;
			if (replayAddr) {
				psidPlayer[1] = psidHdr.defaultTune;
				psidPlayer[3] = psidHdr.loadAddress & 0xff;
				psidPlayer[4] = psidHdr.loadAddress >> 8;
				psidPlayer[22] = psidHdr.replayAddress & 0xff;
				psidPlayer[23] = psidHdr.replayAddress >> 8;
				playerLength = 33;
				playerData = psidPlayer;
			} else {
				rsidPlayer[1] = psidHdr.defaultTune;
				rsidPlayer[2] = 0x4C; // JMP
				rsidPlayer[3] = psidHdr.loadAddress & 0xff;
				rsidPlayer[4] = psidHdr.loadAddress >> 8;
				playerLength = 8;
				playerData = rsidPlayer;
			}
			strcpy(psidHdr.model, "TED8360");

			ted->injectCodeToRAM(psidHdr.loadAddress, tune.getBinaryData(0), dataLen - 2);
			ted->writeProtectedPlayerMemory(playerStartAddress, playerData, playerLength);
		} else { // PRG
			psidHdr.type = -1;
			psidHdr.version = 0;
			psidHdr.tracks = psidHdr.defaultTune = 1;
			psidHdr.current = 1;
			psidHdr.replayAddress = playerStartAddress;
			strcpy(psidHdr.title, fileName);
			strcpy(psidHdr.author, "Unknown");
			strcpy(psidHdr.copyright, "Unknown");
			strcpy(psidHdr.model, "Unknown");
			psidHdr.loadAddress = buf[0] + (buf[1] << 8);

			ted->injectCodeToRAM(psidHdr.loadAddress, buf + 2, bufLength - 2);
			ted->writeProtectedPlayerMemory(playerStartAddress, prgPlayer, sizeof(prgPlayer));
			SIDsound *sid = ted->getSidCard();
			if (sid)
			{
				sid->setModel(SID8580);
				selected_model = SID8580;
			}
		}
#if 1
		cpu->setPC(playerStartAddress);
#else
		char start[64];
		ted->copyToKbBuffer("M\317:\r");
		player->sleep(200);
		sprintf(start, "G%04X\r", playerStartAddress);
		ted->copyToKbBuffer(start);
#endif
		try {
			tedplayPlay();
		} catch (char *txt) {
			std::cerr << "Exception: " << txt << std::endl;
			exit(2);
		}
#if 0 //_DEBUG
		//Sleep(3000);
		if (fileName) {
			std::string fn = fileName;
			fn += ".bin";
			dumpMem(fn);
		}
#endif

		// model override
		if (model >= 0)
		{
			SIDsound *sid = ted->getSidCard();
			if (sid)
			{
				sid->setModel(model);
				selected_model = model;
			}
		}
		return 0;
	}
	return -1;
}

void tedPlaySetVolume(unsigned int masterVolume)
{
	if (ted)
		ted->setMasterVolume(masterVolume);
}

void tedPlaySetSpeed(unsigned int speedPct)
{
	if (ted)
		ted->setplaybackSpeed(speedPct);
}

void tedPlayGetSongs(unsigned int &current, unsigned int &total)
{
	current = psidHdr.current;
	total = psidHdr.tracks;
}

bool tedPlayIsChannelEnabled(unsigned int channel)
{
	return ted->isChannelEnabled(channel);
}

void tedPlayChannelEnable(unsigned int channel, bool enable)
{
	ted->enableChannel(channel, enable);
}

void tedPlaySidEnable(bool enable, unsigned int disableMask)
{
	ted->enableSidCard(enable, disableMask);
}

bool tedPlayCreateWav(const char *fileName)
{
	return false;
}

void tedPlayCloseWav()
{
}
