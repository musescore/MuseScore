/*<empty clipboard>
	YAPE - Yet Another Plus/4 Emulator

	The program emulates the Commodore 264 family of 8 bit microcomputers

	This program is free software, you are welcome to distribute it,
	and/or modify it under certain conditions. For more information,
	read 'Copying'.

	(c) 2000, 2001, 2004, 2005 Attila Grósz
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <ctype.h>
#include "Tedmem.h"
#include "Sid.h"
#include "Cpu.h"
#include "roms.h"
#include "Filter.h"

#define TEXTMODE	0x00000000
#define MULTICOLOR	0x00000010
#define GRAPHMODE	0x00000020
#define EXTCOLOR	0x00000040
#define REVERSE		0x00000080
#define ILLEGAL		0x0000000F

static unsigned int		VertSubCount;
static int				x,tmp;
static unsigned char	*VideoBase;

ClockCycle TED::CycleCounter;
static bool ScreenOn, AttribFetch;
static bool SideBorderFlipFlop, CharacterWindow;
static unsigned int BadLine;
static unsigned int	ClockingState;
static unsigned int	CharacterCount = 0;
static bool VertSubActive;
static unsigned int	CharacterPosition;
static unsigned int	CharacterPositionReload;
static unsigned int	TVScanLineCounter;
static bool HBlanking;
static bool VBlanking;
static bool aligned_write;
static unsigned char *aw_addr_ptr;
static unsigned char aw_value;
static unsigned int ff1d_latch;
TED *TED::instance_;

enum {
	TSS = 1 << 1,
	TDS = 1 << 2,
	TRFSH = 1 << 3,
	THALT1 = 1 << 4,
	THALT2 = 1 << 5,
	THALT3 = 1 << 6,
	TDMA = 1 << 7
};

TED::TED() : filter(0), sidCard(0)
{
	register unsigned int	i;

	instance_ = this;
	// clearing cartdridge ROMs
	for (i=0;i<4;++i) {
		memset(&(RomHi[i]),0,ROMSIZE);
		memset(&(RomLo[i]),0,ROMSIZE);
		memset(romlopath,0,sizeof(romlopath));
		memset(romhighpath,0,sizeof(romhighpath));
	};
	// default ROM sets
	strcpy(romlopath[0],"BASIC");
	strcpy(romhighpath[0],"KERNAL");

	// 64 kbytes of memory allocated
	RAMMask=0xFFFF;

	// actual ram bank pointer default setting
	actram=Ram;

	// setting screen memory pointer
	scrptr=screen;
	// pointer of the end of the screen memory
	endptr=scrptr+456*312-8;
	// setting the CPU to fast mode
	fastmode=1;
	// initial position of the electron beam (upper left corner)
	irqline=VertSubCount=0;
	beamy=0;
	beamx=0;
	hshift = 0;
	scrblank= false;

	charrombank=charrambank=cset=VideoBase=Ram;
	scrattr=0;
	timer1=timer2=timer3=0;
	chrbuf = DMAbuf;
	clrbuf = DMAbuf + 64;
	tmpClrbuf = DMAbuf + 128;
	memset(DMAbuf, 0, sizeof(DMAbuf));

	// create an instance of the keyboard class
//	keys = new KEYS;
//	tap = new TAP;
	// setting the TAP::mem pointer to this MEM class
//	tap->mem=this;
//	tcbmbus = NULL;
	crsrblinkon = false;
	VertSubActive = false;
	CharacterPositionReload = CharacterPosition = 0;
	SideBorderFlipFlop = false;
	render_ok = false;

	BadLine = 0;
	CycleCounter = 0;
	oscillatorInit();
	memset(protectedPlayerMemory, 0xfe, sizeof(protectedPlayerMemory));
	enableSidCard(true, 0);
}

void TED::Reset()
{
	// clear RAM with powerup pattern
	int i;
	for (i = 0; i < RAMSIZE; i++)
		Ram[i] = ((i >> 1) << 1 == i) ? 0 : 0xff;
	// reset oscillators
	oscillatorReset();
	if (sidCard) sidCard->reset();
	lastResetCycle = CycleCounter;
}

void TED::forcedReset()
{
	ChangeMemBankSetup(false);
	Reset();
}

void TED::texttoscreen(int x,int y, const char *scrtxt)
{
	int i = 0;
	
	while (scrtxt[i] != 0)
	{
		chrtoscreen(x + i * 8, y, scrtxt[i]);
		i++;
	}
}

void TED::chrtoscreen(int x,int y, char scrchr)
{
	register int j, k;
	unsigned char *charset = (unsigned char *) kernal+0x1000;

	if (isalpha(scrchr)) {
		scrchr=toupper(scrchr)-64;
		charset+=(scrchr<<3);
		for (j=0;j<8;j++)
			for (k=0;k<8;k++)
				(*(charset+j) & (0x80>>k)) ? screen[(y+j)*456+x+k]=0x00 : screen[(y+j)*456+x+k]=0x71;
		return;
	}
	charset+=(scrchr<<3);
	for (j=0;j<8;j++)
		for (k=0;k<8;k++)
			(*(charset+j) & (0x80>>k)) ? screen[(y+j)*456+x+k]=0x00 : screen[(y+j)*456+x+k]=0x71;
}

void TED::loadroms()
{
	for (int i=0;i<4;i++) {
		loadhiromfromfile(i,romhighpath[i]);
		loadloromfromfile(i,romlopath[i]);
	}
	mem_8000_bfff = actromlo = &(RomLo[0][0]);
	mem_fc00_fcff = mem_c000_ffff = actromhi = &(RomHi[0][0]);
}

void TED::loadloromfromfile(int nr, char fname[512])
{
	FILE			*img;

	if (fname[0]!='\0') {
		if (img = fopen(fname, "rb")) {
			// load low ROM file
			fread(&(RomLo[nr]),ROMSIZE,1,img);
			fclose(img);
			return;
		}
		switch (nr) {
			case 0: memcpy(&(RomLo[0]),basic,ROMSIZE);
				break;
			case 1: if (!strncmp(fname,"3PLUS1LOW",9))
						memcpy(&(RomLo[1]),plus4lo,ROMSIZE);
					else
						memset(&(RomLo[1]),0,ROMSIZE);
				break;
			default : memset(&(RomLo[nr]),0,ROMSIZE);
		}
	} else
		memset(&(RomLo[nr]),0,ROMSIZE);
}

void TED::loadhiromfromfile(int nr, char fname[512])
{
	FILE	*img;

	if (fname[0]!='\0') {
		if (img = fopen(fname, "rb")) {
			// load high ROM file
			fread(&(RomHi[nr]),ROMSIZE,1,img);
			fclose(img);
			return;
		}
		switch (nr) {
			case 0:
				memcpy(&(RomHi[0]),kernal,ROMSIZE);
				break;
			case 1: if (!strncmp(fname,"3PLUS1HIGH",10))
						memcpy(&(RomHi[1]),plus4hi,ROMSIZE);
					else
						memset(&(RomHi[1]),0,ROMSIZE);
				break;
			default : memset(&(RomHi[nr]),0,ROMSIZE);
		}
	} else
		memset(&(RomHi[nr]),0,ROMSIZE);
}

void TED::injectCodeToRAM(unsigned int address, unsigned char *from, size_t len)
{
	unsigned int bytes = (int)((address + len > 0xffff) ? 0xffff - address : len);
	memcpy(actram + (address & 0xffff), from, bytes);
}

void TED::copyToKbBuffer(char *bufferString, unsigned int bufferLength)
{
	unsigned int bufferAddress = 0x0527;
	if (bufferLength == -1) bufferLength = (unsigned int) strlen(bufferString);

	for (unsigned int i=0; i < bufferLength; i++)
		Write( bufferAddress + i, bufferString[i]);
	
	Write(0xEF, bufferLength);
}

ClockCycle TED::GetClockCount()
{
	return CycleCounter;
}

void TED::ChangeMemBankSetup(bool romoff)
{
	if (romoff) {
		mem_8000_bfff = actram + (0x8000 & RAMMask);
		mem_fc00_fcff = mem_c000_ffff = actram + (0xC000 & RAMMask);
	} else {
		mem_8000_bfff = actromlo;
		mem_c000_ffff = actromhi;
		mem_fc00_fcff = &(RomHi[0][0]);
	}
}

unsigned char TED::Read(unsigned int addr)
{
	switch ( addr & 0xF000 ) {
		case 0x0000:
			switch ( addr & 0xFFFF ) {
				case 0:
					return prddr;
				case 1:
					{
						unsigned char retval =
							(
							//ReadBus()&
							0xC0)
							//|(tap->ReadCSTIn(CycleCounter)&0x10)
							;
						return (prp&prddr)|(retval&~prddr);
					}
				default:
					return actram[addr&0xFFFF];
			}
			break;
		case 0x1000:
		case 0x2000:
		case 0x3000:
			return actram[addr&0xFFFF];
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			return actram[addr&RAMMask];
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			return mem_8000_bfff[addr&0x3FFF];
		case 0xC000:
		case 0xD000:
		case 0xE000:
			return mem_c000_ffff[addr&0x3FFF];
		case 0xF000:
			switch ( addr >> 8 ) {
				case 0xFF:
					switch (addr) {
						case 0xFF00 : return timer1&0xFF;
						case 0xFF01 : return timer1>>8;
						case 0xFF02 : return timer2&0xFF;
						case 0xFF03 : return timer2>>8;
						case 0xFF04 : return timer3&0xFF;
						case 0xFF05 : return timer3>>8;
						case 0xFF06 : return Ram[0xFF06];
						case 0xFF07 : return Ram[0xFF07];
						case 0xFF08 : 
#if 0
							if (joyemu)
								  return keys->feedjoy(Ram[0xFF08])&(keys->feedkey(Ram[0xFD30]));
							  else
								  return keys->feedkey(Ram[0xFD30]);
#else
							return 0xFF;
#endif
						case 0xFF09 : return Ram[0xFF09]|(0x25);
						case 0xFF0A : return Ram[0xFF0A]|(0xA0);
						case 0xFF0B : return irqline & 0xFF;
						case 0xFF0C : return ((crsrpos>>8)&0x03)|0xFC;
						case 0xFF0D : return crsrpos&0xFF;
						case 0xFF0E :
						case 0xFF0F :
						case 0xFF10 :
						case 0xFF11 :
						case 0xFF12 :
						case 0xFF13 :
						case 0xFF14 :
							return Ram[addr];
						case 0xFF15 : return ecol[0]|0x80;	// The highest bit is not used and so always 1
						case 0xFF16 : return ecol[1]|0x80;	// A few games (Rockman) used it...
						case 0xFF17 : return ecol[2]|0x80;
						case 0xFF18 : return ecol[3]|0x80;
						case 0xFF19 : return (framecol&0xFF)|0x80;
						case 0xFF1A : return (CharacterPositionReload>>8)&0xFF;
						case 0xFF1B : return CharacterPositionReload&0xFF;
						case 0xFF1C : return (beamy>>8)|0xFE;
						case 0xFF1D : return beamy&0xFF; /// 1-8. bit of the rasterline counter
						case 0xFF1E : return ((98+beamx)<<1)%228; // raster column
						case 0xFF1F : return 0x80|(crsrphase<<3)|VertSubCount;
						default:
							return mem_c000_ffff[addr&0x3FFF];
					}
					break;
				case 0xFE:
					//return tcbmbus->Read(addr);
					//return 0xFE;
					return protectedPlayerMemory[addr & 0xfff];
				case 0xFD:
					switch (addr>>4) {
						case 0xFD0: // RS232
							return 0xFD;
						case 0xFD1: // User port, PIO & 256 RAM expansion
							//return (tap->IsButtonPressed()<<2)^0xFF;
							return 0xFF;
						case 0xFD2: // Speech hardware
						case 0xFD4: // SID Card
						case 0xFD5:
							if (sidCard) {
								return sidCard->read(addr & 0x1f);
							}
							return 0xFD;
						case 0xFD3:
							return Ram[0xFD30];
					}
					return 0xFD;
				case 0xFC:
					return mem_fc00_fcff[addr&0x3FFF];
				default:
					return mem_c000_ffff[addr&0x3FFF];
			}
	}
	//fprintf(stderr,"Unhandled read %04X\n", addr);
	return 0;
}

void TED::UpdateSerialState(unsigned char portVal)
{
	static unsigned char prevVal = 0xC0;

	//if ((prevVal ^ portVal)&8)
//		tap->SetTapeMotor(CycleCounter, portVal&8);

	prevVal = portVal;
}

void TED::Write(unsigned int addr, unsigned char value)
{
	switch (addr&0xF000) {
		case 0x0000:
			switch ( addr & 0xFFFF ) {
				case 0:
					//fprintf(stderr,"$00 write: %02X\n", value);
					prddr = value & 0xDF;
					UpdateSerialState(prddr&~prp);
					return;
				case 1:
					//fprintf(stderr,"$01 write: %02X\n", value);
					prp = value;
					UpdateSerialState(prddr&~prp);
					return;
				default:
					actram[addr&0xFFFF] = value;
			}
			return;
		case 0x1000:
		case 0x2000:
		case 0x3000:
			actram[addr&0xFFFF] = value;
			return;
		case 0xD000:
			if (sidCard) {
				sidCard->write(addr & 0x1f, value);
				sidCard->setFrequency(1);
			}
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xE000:
			actram[addr&RAMMask] = value;
			return;
		case 0xF000:
			switch ( addr >> 8 ) {
				case 0xFF:
					switch (addr) {
						case 0xFF00 :
							t1on=false; // Timer1 disabled
							t1start=(t1start & 0xFF00)|value;
							timer1=(timer1 & 0xFF00)|value;
							return;
						case 0xFF01 :
							t1on=true; // Timer1 enabled
							t1start=(t1start & 0xFF)|(value<<8);
							timer1=(timer1 & 0x00FF)|(value<<8);
							return;
						case 0xFF02 :
							t2on=false; // Timer2 disabled
							timer2=(timer2 & 0xFF00)|value;
							return;
						case 0xFF03 :
							t2on=true; // Timer2 enabled
							timer2=(timer2&0x00FF)|(value<<8);
							return;
						case 0xFF04 :
							t3on=false;  // Timer3 disabled
							timer3=(timer3&0xFF00)|value;
							return;
						case 0xFF05 :
							t3on=true; // Timer3 enabled
							timer3=(timer3&0x00FF)|(value<<8);
							return;
						case 0xFF06 :
							Ram[0xFF06]=value;
							// get vertical offset of screen when smooth scroll
							vshift = value&0x07;
							// Check if screen is turned on
							if (value&0x10 && beamy == 0 && !AttribFetch) {
								AttribFetch = true;
								VertSubCount = 7;
								if (vshift != (beamy&7)) {
									// FIXME: this is actually delayed by one cycle
									if (beamx>4 && beamx<84)
										ClockingState = TSS;
								}
							}
							if ( (Ram[0xFF06] ^ value) & 0x1F) {
								if (AttribFetch) {
									if (vshift == (beamy&7)) {
										BadLine |= 1;
										if (beamx>=3 && beamx<89) {
											unsigned char idleread = Read((cpuptr->getPC()+1)&0xFFFF);
											unsigned int delay = (BadLine & 2) ? 0 : (beamx - 1) >> 1;
											unsigned int invalidcount = (delay > 3) ? 3 : delay;
											unsigned int invalidpos = delay - invalidcount;
											invalidcount = (invalidcount < 40-invalidpos) ? invalidcount : 40-invalidpos;
											unsigned int newdmapos = (invalidpos+invalidcount < 40) ? invalidpos+invalidcount : 40;
											unsigned int newdmacount = 40 - newdmapos ;
											unsigned int oldcount = 40 - newdmacount - invalidcount;
											memcpy( DMAbuf, clrbuf, oldcount);
											memset( DMAbuf+oldcount, idleread, invalidcount);
											memcpy( DMAbuf+oldcount+invalidcount, VideoBase + CharacterCount + oldcount
												+ invalidcount, newdmacount);
											//DoDMA(tmpClrbuf + delay, 0);
											ClockingState = THALT1;
										} else if (beamx<111 && beamx>=89) {
											// FIXME this breaks on FF1E writes
											// swap DMA fetch pointers for colour DMA...
											if (!(BadLine & 1) && beamx < 94) {
												unsigned char *tmpbuf = clrbuf;
												clrbuf = tmpClrbuf;
												tmpClrbuf = tmpbuf;
											}
											//BadLine |= 1;
										}
									} else if (BadLine & 1 /*beamx>=91*/) {
										if (beamx>=94) {
											BadLine &= ~1;
										} else if (beamx >= 91) {
											unsigned char *tmpbuf = clrbuf;
											clrbuf = tmpClrbuf;
											tmpClrbuf = tmpbuf;
											BadLine &= ~1;
										}
									}
								}
							}
							// Delayed DMA?
							// check for flat screen (23 rows)
							fltscr = !(value&0x08);
							// check for extended mode
							ecmode=value&EXTCOLOR;
							if (ecmode) {
								tmp=(0xF800)&RAMMask;
								scrattr|=EXTCOLOR;
							} else {
								tmp=(0xFC00)&RAMMask;
								scrattr&=~EXTCOLOR;
							}
							charrambank=Ram+((Ram[0xFF13]<<8)&tmp);
							charrombank=&(RomHi[0][((Ram[0xFF13] & 0x3C)<<8)&tmp]);
							// check for graphics mode (5th b14it)
							scrattr=(scrattr&~GRAPHMODE)|(value&GRAPHMODE);
							return;
						case 0xFF07 : 
							Ram[0xFF07]=value;
							// check for narrow screen (38 columns)
							nrwscr=value&0x08;
							// get horizontal offset of screen when smooth scroll
							hshift=value&0x07;
							// check for reversed mode
							rvsmode=value&0x80;
							if (rvsmode) {
								tmp=(0xF800)&RAMMask;
								scrattr|=REVERSE;
							} else {
								tmp=(0xFC00)&RAMMask;
								scrattr&=~REVERSE;
							}
							charrombank=&(RomHi[0][((Ram[0xFF13] & 0x3C)<<8)&tmp]);
							charrambank=Ram+((Ram[0xFF13]<<8)&tmp);
							cset = charrom ? charrombank : charrambank;
							// check for multicolor mode
							scrattr=(scrattr&~MULTICOLOR)|(value&0x10);
							return;
						case 0xFF08 :
							Ram[0xFF08] = value;
							return;
						case 0xFF09 :
							// clear the interrupt requester bits
							// by writing 1 into them (!!)
							Ram[0xFF09]=(Ram[0xFF09]&0x7F)&(~value);
							return;
						case 0xFF0A :
							{
								Ram[0xFF0A]=value;
								// change the raster irq line
								unsigned int newirqline = (irqline&0xFF)|((value&0x01)<<8);
								if (newirqline != irqline) {
									if (beamy == newirqline)
										Ram[0xFF0A]&0x02 ? Ram[0xFF09]|=0x82 : Ram[0xFF09]|=0x02;
									irqline = newirqline;
								}
							}
							return;
						case 0xFF0B :
							{
								Ram[0xFF0B]=value;
								unsigned int newirqline = value|(irqline&0x0100);
								if (newirqline != irqline) {
									if (beamy == newirqline)
										Ram[0xFF0A]&0x02 ? Ram[0xFF09]|=0x82 : Ram[0xFF09]|=0x02;
									irqline = newirqline;
								}
							}
							return;
						case 0xFF0C :
							crsrpos=((value<<8)|(crsrpos&0xFF))&0x3FF;
							return;
						case 0xFF0D :
							crsrpos=value|(crsrpos&0xFF00);
							return;
						case 0xFF0E :
							writeSoundReg(0, value);
							Ram[0xFF0E]=value;
							return;
						case 0xFF0F :
							writeSoundReg(1, value);
							Ram[0xFF0F]=value;
							return;
						case 0xFF10 :
							writeSoundReg(2, value & 3);
							Ram[0xFF10]=value;
							return;
						case 0xFF11 :
							Ram[0xFF11]=value;
							writeSoundReg(3, value);
							return;
						case 0xFF12:
							grbank=Ram+((value&0x38)<<10);
							if ((value ^ Ram[0xFF12]) & 3)
								writeSoundReg(4, value & 3);
							// if the 2nd bit is set the chars are read from ROM
							charrom=(value&0x04)>0;
							if (charrom && Ram[0xFF13]<0x80)
								scrattr|=ILLEGAL;
							else {
								scrattr&=~ILLEGAL;
								cset = (charrom) ? charrombank : charrambank;
							}
							Ram[0xFF12]=value;
							return;
						case 0xFF13 :
							// the 0th bit is not writable, it indicates if the ROMs are on
							Ram[0xFF13]=(value&0xFE)|(Ram[0xFF13]&0x01);
							// bit 1 is the fast/slow mode switch
							fastmode = !(value&0x02);
							(ecmode || rvsmode) ? tmp=(0xF800)&RAMMask : tmp=(0xFC00)&RAMMask;
							charbank = ((value)<<8)&tmp;
							charrambank=Ram+charbank;
							charrombank=&(RomHi[0][charbank & 0x3C00]);
							if (charrom && value<0x80)
								scrattr|=ILLEGAL;
							else {
								scrattr&=~ILLEGAL;
								(charrom) ? cset = charrombank : cset = charrambank;
							}
							return;
						case 0xFF14 :
							Ram[0xFF14]=value;
							VideoBase=colorbank=Ram+(((value&0xF8)<<8)&RAMMask);
							return;
						case 0xFF15 :
							ecol[0]=bmmcol[0]=mcol[0]=value&0x7F;
							return;
						case 0xFF16 :
							ecol[1]=bmmcol[3]=mcol[1]=value&0x7F;
							return;
						case 0xFF17 :
							ecol[2]=mcol[2]=value&0x7F;
							return;
						case 0xFF18 :
							ecol[3]=value&0x7F;
							return;
						case 0xFF19 :
							value &= 0x7F;
							framecol=(value<<24)|(value<<16)|(value<<8)|value;
							return;
						case 0xFF1A :
							CharacterPositionReload = (CharacterPositionReload & 0xFF) | ((value&3)<<8);
							return;
						case 0xFF1B :
							CharacterPositionReload = (CharacterPositionReload & 0x300) | value;
							return;
						case 0xFF1C :
							beamy=((value&0x01)<<8)|(beamy&0xFF);
							return;
						case 0xFF1D :
							beamy=(beamy&0x0100)|value;
							return;
						case 0xFF1E :
							{
								unsigned int low_x = beamx&1;
								// lowest 2 bits are not writable
								// inverted value must be written
								unsigned int new_beamx=((~value))&0xFC;
								new_beamx >>= 1;
								new_beamx>=98 ?  new_beamx -= 98 : new_beamx += 16;
								// writes are aligned to single clock cycles
								if (low_x) {
									aligned_write = true;
									aw_addr_ptr = (unsigned char*)(&beamx);
									aw_value = new_beamx;
								} else {
									beamx = new_beamx;
								}
							}
							return;
						case 0xFF1F :
							VertSubCount=value&0x07;
							crsrphase=(value&0x78)>>3;
							return;
						case 0xFF3E :
							Ram[0xFF13]|=0x01;
							RAMenable=false;
							ChangeMemBankSetup(RAMenable);
							return;
						case 0xFF3F :
							Ram[0xFF13]&=0xFE;
							RAMenable=true;
							ChangeMemBankSetup(RAMenable);
							return;
					}
					actram[addr&RAMMask] = value;
					return;
				case 0xFE:
					//tcbmbus->Write(addr,value);
					return;
				case 0xFD:
					switch (addr>>4) {
						case 0xFD0: // RS232
						case 0xFD1: // User port, PIO & 256 RAM expansion
						case 0xFD2: // Speech hardware
							return;
						case 0xFD3:
							Ram[0xFD30] = value;
							return;
						case 0xFD4: // SID Card
						case 0xFD5:
							if (sidCard) {
								sidCard->setFrequency(0);
								sidCard->write(addr & 0x1f, value);
							}
							return;
						case 0xFDD:
							actromlo=&(RomLo[addr&0x03][0]);
							actromhi=&(RomHi[(addr&0x0c)>>2][0]);
							return;
					}
					return;
				default:
					actram[addr&RAMMask] = value;
					return;
			}
	}
	return;
}

void TED::dump(void *img)
{
	// this is ugly :-P
   	fwrite(Ram,RAMSIZE,1,(FILE *) img);
	fwrite(&RAMenable,sizeof(RAMenable),1,(FILE *) img);
	fwrite(&t1start,sizeof(t1start),1, (FILE *) img);
	fwrite(&t1on,sizeof(t1on),1, (FILE *) img);
	fwrite(&t2on,sizeof(t2on),1, (FILE *) img);
	fwrite(&t3on,sizeof(t3on),1, (FILE *) img);
	fwrite(&timer1,sizeof(timer1),1, (FILE *) img);
	fwrite(&timer2,sizeof(timer2),1, (FILE *) img);
	fwrite(&timer3,sizeof(timer3),1, (FILE *) img);
	fwrite(&beamx,sizeof(beamx),1, (FILE *) img);
	fwrite(&beamy,sizeof(beamy),1, (FILE *) img);
	//fwrite(&x,sizeof(x),1, (FILE *) img);
	fwrite(&irqline,sizeof(irqline),1, (FILE *) img);
	fwrite(&crsrpos,sizeof(crsrpos),1, (FILE *) img);
	fwrite(&scrattr,sizeof(scrattr),1, (FILE *) img);
	fwrite(&tmp,sizeof(tmp),1, (FILE *) img);
	fwrite(&nrwscr,sizeof(nrwscr),1, (FILE *) img);
	fwrite(&hshift,sizeof(hshift),1, (FILE *) img);
	fwrite(&vshift,sizeof(vshift),1, (FILE *) img);
	fwrite(&fltscr,sizeof(fltscr),1, (FILE *) img);
	fwrite(&mcol,sizeof(mcol),1, (FILE *) img);
	fwrite(chrbuf,40,1, (FILE *) img);
	fwrite(clrbuf,40,1, (FILE *) img);
	fwrite(&charrom,sizeof(charrom),1, (FILE *) img);
	fwrite(&charbank,sizeof(charbank),1, (FILE *) img);
//	fwrite(&TEDfreq1,sizeof(TEDfreq1),1, (FILE *) img);
//	fwrite(&TEDfreq2,sizeof(TEDfreq2),1, (FILE *) img);
//	fwrite(&TEDVolume,sizeof(TEDVolume),1, (FILE *) img);
//	fwrite(&TEDDA,sizeof(TEDDA),1, (FILE *) img);
	fwrite(&framecol,sizeof(framecol),1, (FILE *) img);
}

void TED::memin(void *img)
{

	// this is ugly :-P
   	fread(Ram,RAMSIZE,1,(FILE *) img);

	fread(&RAMenable,sizeof(RAMenable),1,(FILE *) img);
	fread(&t1start,sizeof(t1start),1, (FILE *) img);
	fread(&t1on,sizeof(t1on),1, (FILE *) img);
	fread(&t2on,sizeof(t2on),1, (FILE *) img);
	fread(&t3on,sizeof(t3on),1, (FILE *) img);
	fread(&timer1,sizeof(timer1),1, (FILE *) img);
	fread(&timer2,sizeof(timer2),1, (FILE *) img);
	fread(&timer3,sizeof(timer3),1, (FILE *) img);
	fread(&beamx,sizeof(beamx),1, (FILE *) img);
	fread(&beamy,sizeof(beamy),1, (FILE *) img);
	//fread(&x,sizeof(x),1, (FILE *) img);
	fread(&irqline,sizeof(irqline),1, (FILE *) img);
	fread(&crsrpos,sizeof(crsrpos),1, (FILE *) img);
	fread(&scrattr,sizeof(scrattr),1, (FILE *) img);
	fread(&tmp,sizeof(tmp),1, (FILE *) img);
	fread(&nrwscr,sizeof(nrwscr),1, (FILE *) img);
	fread(&hshift,sizeof(hshift),1, (FILE *) img);
	fread(&vshift,sizeof(vshift),1, (FILE *) img);
	fread(&fltscr,sizeof(fltscr),1, (FILE *) img);
	fread(&mcol,sizeof(mcol),1, (FILE *) img);
	fread(chrbuf,40,1, (FILE *) img);
	fread(clrbuf,40,1, (FILE *) img);
	fread(&charrom,sizeof(charrom),1, (FILE *) img);
	fread(&charbank,sizeof(charbank),1, (FILE *) img);
//	fread(&TEDfreq1,sizeof(TEDfreq1),1, (FILE *) img);
//	fread(&TEDfreq2,sizeof(TEDfreq2),1, (FILE *) img);
//	fread(&TEDVolume,sizeof(TEDVolume),1, (FILE *) img);
//	fread(&TEDDA,sizeof(TEDDA),1, (FILE *) img);
	fread(&framecol,sizeof(framecol),1, (FILE *) img);

	for (int i=0; i<5; i++)
		writeSoundReg(i, Ram[0xFF0E + i]);
	beamy=0;
	beamx=0;
	scrptr=&(screen[0]);
	charrambank=Ram+charbank;
	charrombank=&(RomHi[0][charbank & 0x3C00]);
	(charrom) ? cset = charrombank : cset = charrambank;

}

// when multi and extended color modes are all on the screen is blank
inline void TED::mcec()
{
	memset( scrptr, 0, 8);
}

// renders hires text with reverse (128 chars)
inline void TED::hi_text()
{
    unsigned char	chr;
	unsigned char	charcol;
	unsigned char	mask;
	unsigned char	*wbuffer = scrptr + hshift;

	// get the actual physical character column
	charcol=clrbuf[x];
	chr=chrbuf[x];

	if ((charcol)&0x80 && !crsrblinkon)
		mask = 00;
	else
		mask = cset[((chr&0x7F)<<3)|VertSubCount];

	if (chr&0x80)
		mask ^= 0xFF;
	if (crsrpos==((CharacterPosition+x)&0x3FF) && crsrblinkon )
		mask ^= 0xFF;

	wbuffer[0] = (mask & 0x80) ? charcol : mcol[0];
	wbuffer[1] = (mask & 0x40) ? charcol : mcol[0];
	wbuffer[2] = (mask & 0x20) ? charcol : mcol[0];
	wbuffer[3] = (mask & 0x10) ? charcol : mcol[0];
	wbuffer[4] = (mask & 0x08) ? charcol : mcol[0];
	wbuffer[5] = (mask & 0x04) ? charcol : mcol[0];
	wbuffer[6] = (mask & 0x02) ? charcol : mcol[0];
	wbuffer[7] = (mask & 0x01) ? charcol : mcol[0];
}

// renders text without the reverse (all 256 chars)
inline void TED::rv_text()
{
	unsigned char	chr;
	unsigned char	charcol;
	unsigned char	mask;
	unsigned char	*wbuffer = scrptr + hshift;

	// get the actual physical character column
	charcol=clrbuf[x];
	chr=chrbuf[x];

	if ((charcol)&0x80 && !crsrblinkon)
		mask = 00;
	else
		mask = cset[(chr<<3)|VertSubCount];

	if (crsrpos==((CharacterPosition+x)&0x3FF) && crsrblinkon )
		mask ^= 0xFF;

	wbuffer[0] = (mask & 0x80) ? charcol : mcol[0];
	wbuffer[1] = (mask & 0x40) ? charcol : mcol[0];
	wbuffer[2] = (mask & 0x20) ? charcol : mcol[0];
	wbuffer[3] = (mask & 0x10) ? charcol : mcol[0];
	wbuffer[4] = (mask & 0x08) ? charcol : mcol[0];
	wbuffer[5] = (mask & 0x04) ? charcol : mcol[0];
	wbuffer[6] = (mask & 0x02) ? charcol : mcol[0];
	wbuffer[7] = (mask & 0x01) ? charcol : mcol[0];
}

// renders extended color text
inline void TED::ec_text()
{
	unsigned char charcol;
	unsigned char chr;
	unsigned char mask;
	unsigned char *wbuffer = scrptr + hshift;

	// get the actual physical character column
	charcol = clrbuf[x];
	chr = chrbuf[x];

	mask = cset[ ((chr&0x3F)<<3)|VertSubCount ];

	chr = (chr&0xC0)>>6;

	wbuffer[0] = (mask & 0x80) ? charcol : ecol[chr];
	wbuffer[1] = (mask & 0x40) ? charcol : ecol[chr];
	wbuffer[2] = (mask & 0x20) ? charcol : ecol[chr];
	wbuffer[3] = (mask & 0x10) ? charcol : ecol[chr];
	wbuffer[4] = (mask & 0x08) ? charcol : ecol[chr];
	wbuffer[5] = (mask & 0x04) ? charcol : ecol[chr];
	wbuffer[6] = (mask & 0x02) ? charcol : ecol[chr];
	wbuffer[7] = (mask & 0x01) ? charcol : ecol[chr];
}

// renders multicolor text
inline void TED::mc_text_rvs()
{
	unsigned char chr=chrbuf[x];
	unsigned char charcol=clrbuf[x];
	unsigned char *wbuffer = scrptr + hshift;
	unsigned char mask;

	mask = cset[ (chr<<3)|VertSubCount ];

	if (charcol&0x08) { // if character is multicolored

		mcol[3]=charcol&0xF7;

		wbuffer[0] = wbuffer[1] = mcol[ (mask & 0xC0) >> 6 ];
		wbuffer[2] = wbuffer[3] = mcol[ (mask & 0x30) >> 4 ];
		wbuffer[4] = wbuffer[5] = mcol[ (mask & 0x0C) >> 2 ];
		wbuffer[6] = wbuffer[7] = mcol[  mask & 0x03 ];

	} else { // this is a normally colored character

		wbuffer[0] = (mask & 0x80) ? charcol : mcol[0];
		wbuffer[1] = (mask & 0x40) ? charcol : mcol[0];
		wbuffer[2] = (mask & 0x20) ? charcol : mcol[0];
		wbuffer[3] = (mask & 0x10) ? charcol : mcol[0];
		wbuffer[4] = (mask & 0x08) ? charcol : mcol[0];
		wbuffer[5] = (mask & 0x04) ? charcol : mcol[0];
		wbuffer[6] = (mask & 0x02) ? charcol : mcol[0];
		wbuffer[7] = (mask & 0x01) ? charcol : mcol[0];
	}
}

// renders multicolor text with reverse bit set
inline void TED::mc_text()
{
	unsigned char charcol=clrbuf[x];
	unsigned char chr=chrbuf[x]&0x7F;
	unsigned char *wbuffer = scrptr + hshift;
	unsigned char mask;

	mask = cset[ (chr<<3)|VertSubCount ];

	if ((charcol)&0x08) { // if character is multicolored

		mcol[3]=(charcol)&0xF7;

		wbuffer[0] = wbuffer[1] = mcol[ (mask & 0xC0) >> 6 ];
		wbuffer[2] = wbuffer[3] = mcol[ (mask & 0x30) >> 4 ];
		wbuffer[4] = wbuffer[5] = mcol[ (mask & 0x0C) >> 2 ];
		wbuffer[6] = wbuffer[7] = mcol[ mask & 0x03 ];

	} else { // this is a normally colored character

		wbuffer[0] = (mask & 0x80) ? charcol : mcol[0];
		wbuffer[1] = (mask & 0x40) ? charcol : mcol[0];
		wbuffer[2] = (mask & 0x20) ? charcol : mcol[0];
		wbuffer[3] = (mask & 0x10) ? charcol : mcol[0];
		wbuffer[4] = (mask & 0x08) ? charcol : mcol[0];
		wbuffer[5] = (mask & 0x04) ? charcol : mcol[0];
		wbuffer[6] = (mask & 0x02) ? charcol : mcol[0];
		wbuffer[7] = (mask & 0x01) ? charcol : mcol[0];
	}
}

// renders hires bitmap graphics
inline void TED::hi_bitmap()
{
	unsigned char mask;
	unsigned char *wbuffer = scrptr + hshift;
	// get the actual color attributes
	hcol[0]=(chrbuf[x]&0x0F)|(clrbuf[x]&0x70);
	hcol[1]=((chrbuf[x]&0xF0)>>4)|((clrbuf[x]&0x07)<<4);

	mask = grbank[ (((CharacterPosition+x)<<3)&0x1FFF)|VertSubCount ];

	wbuffer[0] = (mask & 0x80) ? hcol[1] : hcol[0];
	wbuffer[1] = (mask & 0x40) ? hcol[1] : hcol[0];
	wbuffer[2] = (mask & 0x20) ? hcol[1] : hcol[0];
	wbuffer[3] = (mask & 0x10) ? hcol[1] : hcol[0];
	wbuffer[4] = (mask & 0x08) ? hcol[1] : hcol[0];
	wbuffer[5] = (mask & 0x04) ? hcol[1] : hcol[0];
	wbuffer[6] = (mask & 0x02) ? hcol[1] : hcol[0];
	wbuffer[7] = (mask & 0x01) ? hcol[1] : hcol[0];

}

// renders multicolor bitmap graphics
inline void TED::mc_bitmap()
{
	unsigned char	mask;
	unsigned char	*wbuffer = scrptr + hshift;
	// get the actual color attributes
	bmmcol[1]=((chrbuf[x]&0xF0)>>4)|((clrbuf[x]&0x07)<<4);
	bmmcol[2]=(chrbuf[x]&0x0F)|(clrbuf[x]&0x70);

	mask = grbank[ (((CharacterPosition+x)<<3)&0x1FFF)+VertSubCount ];

	wbuffer[0]= wbuffer[1] = bmmcol[ (mask & 0xC0) >> 6 ];
	wbuffer[2]= wbuffer[3] = bmmcol[ (mask & 0x30) >> 4 ];
	wbuffer[4]= wbuffer[5] = bmmcol[ (mask & 0x0C) >> 2 ];
	wbuffer[6]= wbuffer[7] = bmmcol[  mask & 0x03 ];
}

// "illegal" mode: when $FF13 points to an illegal ROM address
//  the current data on the bus is displayed
inline void TED::illegalbank()
{
	unsigned char	chr = chrbuf[x];
	unsigned char	charcol = clrbuf[x];
	unsigned char	mask;
	unsigned char	*wbuffer = scrptr + hshift;

	if (charcol&0x80 && crsrblinkon)
		mask = 00;
	else {
		if (BadLine==1)
			mask = clrbuf[x];
		else if (BadLine==2)
			mask = chrbuf[x];
		else
			mask = Read(cpuptr->getPC());
	}

	if (chr&0x80)
		mask ^= 0xFF;
	if (crsrpos==((CharacterPosition+x)&0x3FF) && crsrblinkon)
		mask ^= 0xFF;

	wbuffer[0] = (mask & 0x80) ? charcol : mcol[0];
	wbuffer[1] = (mask & 0x40) ? charcol : mcol[0];
	wbuffer[2] = (mask & 0x20) ? charcol : mcol[0];
	wbuffer[3] = (mask & 0x10) ? charcol : mcol[0];
	wbuffer[4] = (mask & 0x08) ? charcol : mcol[0];
	wbuffer[5] = (mask & 0x04) ? charcol : mcol[0];
	wbuffer[6] = (mask & 0x02) ? charcol : mcol[0];
	wbuffer[7] = (mask & 0x01) ? charcol : mcol[0];
}

inline void TED::DoDMA( unsigned char *Buf, unsigned int Offset )
{
	if ( CharacterCount>=0x03D8 ) {
		memcpy( Buf, VideoBase + CharacterCount + Offset, 0x400 - CharacterCount);
		memcpy( Buf + 0x400 - CharacterCount, VideoBase + Offset, (CharacterCount + 40)&0x03FF);
	} else {
		memcpy( Buf, VideoBase + CharacterCount + Offset, 40);
	}
}

extern int currtime;

// main loop of the whole emulation as the TED feeds the CPU with clock cycles
void TED::ted_process(unsigned int count)
{
    do {
        switch(++beamx) {

            default:
                break;

            case 2:
                if (VertSubActive)
                	VertSubCount = (VertSubCount+1)&7;
                break;

            case 3:
				if (AttribFetch && beamy==0) {
					VertSubCount = 7;
				}
				break;

             case 4:
                 if (AttribFetch) {
					BadLine |= ((vshift)&7) == (beamy&7);
					if ( BadLine ) {
						ClockingState = THALT1;
					} else
                 		ClockingState = TSS;
                 	if (beamy==203) {
						AttribFetch = false;
						if (!(BadLine & 2)) ClockingState = TSS;
					}
				}
                break;

            case 8:
				HBlanking = false;
				break;

            case 10:
				if (VertSubActive)
					CharacterPosition = CharacterPositionReload;
                break;

            case 16:
				if (ScreenOn) {
					SideBorderFlipFlop = true;
					memset( scrptr, mcol[0], hshift);
					if (nrwscr)
						CharacterWindow = true;
					x = 0;
				}
				if (BadLine & 1)
					DoDMA(tmpClrbuf, 0);
				if (BadLine & 2)
					DoDMA(chrbuf, (BadLine & 1) ? 0 : 0x400);
                break;

			case 18:
				if (ScreenOn && !nrwscr) {
					CharacterWindow = true;
				}
				break;

			case 89:
				if (/*VertSubActive &&*/ VertSubCount == 6)
					CharacterCount = (CharacterCount + 40)&0x3FF;
				break;

            case 90:
    			if ( VertSubActive && VertSubCount == 7 )
					CharacterPositionReload = (CharacterPosition + 40)&0x3FF;
				break;

            case 91:
		    	ClockingState = TRFSH;
		    	break;

			case 94:
				if (ScreenOn && !nrwscr)
  					SideBorderFlipFlop = CharacterWindow = false;
				break;

  			case 96:
				if (ScreenOn && nrwscr)
  					SideBorderFlipFlop = CharacterWindow = false;
				// FIXME this breaks on FF1E writes
				if (BadLine & 1) {
					// swap DMA fetch pointers for colour DMA...
					unsigned char *tmpbuf = clrbuf;
					clrbuf = tmpClrbuf;
					tmpClrbuf = tmpbuf;
				}
  			    break;

			case 111:
				if ((BadLine&1)) {
    			    BadLine = 2;
					VertSubActive = true;
				} else if (BadLine&2) {// in the second bad line, we're finished...
   					BadLine &= ~2;
				}
				break;

            case 102:
		    	ClockingState = fastmode ? TDS : TSS;
		    	break;

		    case 104:
				HBlanking = true;
				break;

		    case 107: // HSYNC start
				break;

			case 256:
			case 128:
				beamx = 15;
				break;
			case 114: // HSYNC end
    			beamx=0;
    			// the beam reached a new line
    			TVScanLineCounter += 1;
    			if ( TVScanLineCounter >= 340 ) {
					TVScanLineCounter = 0;
					scrptr=screen;
//					render_ok=true;
				}
    			switch (++beamy) {

    			    case 4:
    			        if (!fltscr) ScreenOn = AttribFetch;
    			        break;

    			    case 8:
						if (fltscr) ScreenOn = AttribFetch;
    			        break;

					case 200:
						if (fltscr) ScreenOn = false;
						break;

    			    case 204:
						if (!fltscr) ScreenOn = false;
						break;

    			    case 205:
						CharacterCount = 0;
					    // cursor phase counter in TED register $1F
						VertSubActive = false;
						if ((++crsrphase&0x0F) == 0x0F)
    			        	crsrblinkon ^= 1;
    			        break;

					case 251:
						VBlanking = true;
						break;

    			    case 261: // Vertical retrace
						// frame ready...
						//render_ok=true;
						// reset screen pointer ("TV" electron beam)
						scrptr=screen;
						TVScanLineCounter = 0;
						break;

					case 271:
						VBlanking = false;
						break;

					case 512:
					case 312:
					    beamy = 0;
					    CharacterPosition = CharacterPositionReload = 0;
					    AttribFetch = (Ram[0xFF06]&0x10) != 0;
				}
				// is there raster interrupt?
				if (beamy == irqline)
					Ram[0xFF09] |= Ram[0xFF0A]&0x02 ? 0x82 : 0x02;
		}

		if (beamx&1) {	// perform these only in every second cycle
			if (t2on && !((timer2--)&0xFFFF)) {// Timer2 permitted
				timer2=0xFFFF;
				Ram[0xFF09] |= Ram[0xFF0A]&0x10 ? 0x90 : 0x10; // interrupt
			}
			if (t3on && !((timer3--)&0xFFFF)) {// Timer3 permitted
				timer3=0xFFFF;
				Ram[0xFF09] |= Ram[0xFF0A]&0x40 ? 0xC0 : 0x40; // interrupt
			}
			if (!CharacterWindow && !HBlanking && !VBlanking) {
				// we are on the border area, so use the frame color
				*((int*)(scrptr+4)) = framecol;
			}
			if (scrptr != endptr)
				scrptr+=8;
		} else {
			if (t1on && !timer1--) { // Timer1 permitted decreased and zero
				timer1=(t1start-1)&0xFFFF;
				Ram[0xFF09] |= Ram[0xFF0A]&0x08 ? 0x88 : 0x08; // interrupt
			}

			if (!(HBlanking |VBlanking)) {
				if (SideBorderFlipFlop) { // drawing the visible part of the screen
					// call the relevant rendering function
					//switch (scrattr) {
					//	case 0:
					//		hi_text();
					//		break;
					//	case REVERSE :
					//		rv_text();
					//		break;
					//	case MULTICOLOR|REVERSE :
					//	    mc_text_rvs();
					//	    break;
					//	case MULTICOLOR :
					//		mc_text();
					//		break;
					//	case EXTCOLOR|REVERSE :
					//	case EXTCOLOR :
					//		ec_text();
					//		break;
					//	case GRAPHMODE|REVERSE :
					//	case GRAPHMODE :
					//		hi_bitmap();
					//		break;
					//	case EXTCOLOR|MULTICOLOR :
					//	case GRAPHMODE|EXTCOLOR :
					//	case GRAPHMODE|MULTICOLOR|EXTCOLOR :
					//	case REVERSE|MULTICOLOR|EXTCOLOR :
					//	case GRAPHMODE|MULTICOLOR|EXTCOLOR|REVERSE :
					//		mcec();
					//		break;
					//	case GRAPHMODE|MULTICOLOR :
					//	case GRAPHMODE|MULTICOLOR|REVERSE :
					//		mc_bitmap();
					//		break;
					//	default:
				 //   		illegalbank();
				 //   		break;
					//}
					x = (x + 1) & 0x3F;
				}
				if (!CharacterWindow) {
					// we are on the border area, so use the frame color
					*((int*)scrptr) = framecol;
				}
			}

			CycleCounter++;
			if (!(CycleCounter & 0x03) && int(count) > 0) {
				static unsigned int remainder = 0;
				unsigned int samples = (playbackSpeed + remainder) / 4;
				if (samples) {
					renderSound(samples);
					if (sidCard) {
						sidCard->calcSamples(samples);
					}
				}
				remainder = (playbackSpeed + remainder) % 4;
				count -= samples;
				currtime += samples;
			}
		}
		if (aligned_write) {
			*aw_addr_ptr = aw_value;
			aligned_write = false;
		}
		switch (ClockingState|(beamx&1)) {
        	case TRFSH|1:
 	    	case TSS|1:
 	    	case TDS|1:
       		case TDS:
     	    	cpuptr->process();
     	    	break;
      		case THALT1|1:
  	    	case THALT2|1:
        	case THALT3|1:
            	cpuptr->stopcycle();
            	ClockingState<<=1;
            	break;
		}

	} while (int(count) > 0);

	render_ok = false;
}

void TED::enableChannel(unsigned int channel, bool enable) 
{
	channelMask[channel % 3] = enable ? -1 : 0;
	if (sidCard) sidCard->enableDisableChannel(channel, enable);
}

void TED::enableSidCard(bool enable, unsigned int disableMask)
{
	if (enable) {
		if (sidCard)
			return;
		sidCard = new SIDsound(SID8580, disableMask);
		sidCard->setSampleRate(TED_SOUND_CLOCK);
	} else {
		if (!sidCard)
			return;
		delete sidCard;
		sidCard = 0;
	}
}

TED::~TED()
{
}
//--------------------------------------------------------------

