#pragma once

#include <string>
#include "psid.h"

class TED;

extern TED *machineInit();
extern void tedPlayGetInfo(void *file, PsidHeader &hdr);
extern int tedplayMain(char *fileName, int model);
extern void tedPlaySetVolume(unsigned int masterVolume);
extern void tedPlaySetSpeed(unsigned int speedPct);
extern void tedplayPause();
extern void tedplayPlay();
extern void tedplayStop();
extern void tedplayClose();
extern void tedPlayGetSongs(unsigned int &current, unsigned int &total);
extern bool tedPlayIsChannelEnabled(unsigned int channel);
extern void tedPlayChannelEnable(unsigned int channel, bool enable);
extern int tedPlayGetState();
extern void tedPlaySidEnable(bool enable, unsigned int disableMask);
extern void tedPlaySetFilterOrder(unsigned int filterOrder);
extern unsigned int tedPlayGetWaveform(unsigned int channel);
extern void tedPlaySetWaveform(unsigned int channel, unsigned int wave);
extern bool tedPlayCreateWav(const char *fileName);
extern void tedPlayCloseWav();
extern unsigned int tedplayGetSecondsPlayed();
extern void tedPlayResetCycleCounter();
//
extern void machineReset();
extern void machineDoSomeFrames(unsigned int count);
extern void dumpMem(std::string name);
