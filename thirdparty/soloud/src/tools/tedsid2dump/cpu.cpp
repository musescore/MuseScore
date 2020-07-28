#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "cpu.h"
#include "tedmem.h"

//#define CPU_STATS 
#ifdef CPU_STATS 
#include "Cpu7501asm.h"
#endif

// this one is much quicker
#define push(VALUE) (stack[SP--]=(VALUE))
#define pull() (stack[SP])

bool CPU::bp_active = false;
bool CPU::bp_reached = false;
const unsigned int CPU::nr_of_bps = 11;
static unsigned int stats[255];

CPU::CPU( MemoryHandler *memhandler, unsigned char *irqreg, unsigned char *cpustack ) : mem(memhandler), irq_register(irqreg), stack(cpustack)
{
	irq_sequence = 0;
	IRQcount = 0;
	cpu_jammed = false;
	PC = 0xFFFF;
	for( int i=0; i<nr_of_bps; i++) {
		bp[i].address = 0x10000;
		bp[i].enabled = false;
		bp[i].slot_free = true;
	}
	nr_activebps = 0;
	currins = 0;
	nextins = 0;
	farins = 0;
	SP = 0;
	ST = 0;
	AC = 0;
	X = 0;
	Y = 0;
	cycle = 0;
	ptr = 0;
	memset( stats, 0, sizeof(stats));
}

unsigned int CPU::getcins()
{ 
	// currins is set to 0x00 when in IRQ so we have to re-fetch the opcode
	// this is only used for debugging & co.
	return (currins==0) ?  mem->Read((PC-1)&0xFFFF) : currins&0xFF;
};

bool CPU::saveshot(void *CPUdump)
{
	FILE *img = (FILE *) CPUdump;

	fwrite(&currins,sizeof(currins),1,img);
	fwrite(&nextins,sizeof(nextins),1,img);
	fwrite(&ptr,sizeof(ptr),1,img);
	fwrite(&PC,sizeof(PC),1,img);
	fwrite(&cycle,sizeof(cycle),1,img);
	fwrite(&SP,sizeof(SP),1,img);
  	fwrite(&ST,sizeof(ST),1,img);
   	fwrite(&AC,sizeof(AC),1,img);
   	fwrite(&X,sizeof(X),1,img);
   	fwrite(&Y,sizeof(Y),1,img);
	return true;
}

bool CPU::loadshot(void *CPUdump)
{
	FILE *img = (FILE *) CPUdump;

	fread(&currins,sizeof(currins),1,img);
	fread(&nextins,sizeof(nextins),1,img);
	fread(&ptr,sizeof(ptr),1,img);
   	fread(&PC,sizeof(PC),1,img);
	PC &= 0xFFFF;
	fread(&cycle,sizeof(cycle),1,img);
   	fread(&SP,sizeof(SP),1,img);
  	fread(&ST,sizeof(ST),1,img);
   	fread(&AC,sizeof(AC),1,img);
   	fread(&X,sizeof(X),1,img);
   	fread(&Y,sizeof(Y),1,img);
	return true;
}

// reset keeps all except for PC
void CPU::Reset()
{
	ST=0x24;
	softreset();
	IRQcount = cycle = 0;
	irq_sequence = 0;
}

void CPU::softreset()
{
	// clear the BRK flag
	ST&=0xEF;
	setPC(mem->Read(0xFFFC)|(mem->Read(0xFFFD)<<8));
}

void CPU::setPC(unsigned int addr)
{
	PC=addr;
	cycle=0;
}

inline void CPU::DoCompare(unsigned char reg, unsigned char value)
{
	ST = (ST & 0xFE) | ( value<=reg);
	SETFLAGS_ZN( reg - value);
}

void CPU::process()
{
	if (IRQcount || ((*irq_register)&0x80 ) && !irq_sequence && !(ST&0x04))
		IRQcount++;

	if (!cycle) {		// in the first cycle the CPU fetches the opcode
		if (IRQcount>=3 && currins != 0x58) {
			IRQcount = 0;
			if (!(ST&0x04) || currins == 0x78) { //  
				irq_sequence = 0x10; // this is 0x10 to match the B flag later
				currins = 0x00;	 // we could use BRK for the IRQ routine!
				cycle = 1;
				return;
			}
		}
		currins=mem->Read(PC);				// fetch opcode
		nextins=mem->Read(PC+1);			// prefetch next opcode/operand
		cycle = 1;							// increment the CPU cycle counter
#ifdef CPU_STATS
		stats[currins]++;
		static FILE *f = fopen("disasm.txt", "a");
		//unsigned char t = mem->Read(PC+1);
		if (f)
		{
			fprintf(f, ". %04X %s", PC, ins[currins].name);
			int i = 1;
			while (i < typlen[ins[currins].type]) {
				fprintf(f, " %02X", mem->Read(PC+i++));
			}
			fprintf(f, "\n");
			fflush(f);
		}
#endif
		PC=(PC+1)&0xFFFF;
	}
	else 

	switch (currins){
		case 0xea : // NOP
			cycle=0;
			break;

		case 0x18 : // CLC
			ST&=0xFE;
			cycle=0;
			break;

		case 0x38 : // SEC
			ST|=0x01;
			cycle=0;
			break;

		case 0x58 : // CLI
			ST&=0xFB;
			cycle=0;
			break;

		case 0x78 : // SEI
			ST|=0x04;
			cycle=0;
			// actually, IRQ can still occur right after the SEI instruction
			// some programs (like Castle Master) rely on it.
			// Therefore startIRQ is not set to 0 here.
			//IRQcount = 0;
			break; 

		case 0xb8 : // CLV
			// This has to be more generic because the drive byte ready
			// line is connected to the V flag pin on the disk drive CPU
			ClearVFlag();
			cycle=0;
			break;

		case 0xD8 : // CLD
			ST&=0xF7;
			cycle=0;
			break;

		case 0xF8 : // SED
			ST|=0x08;
			cycle=0;
			break;

		// branch functions (conditional jumps )

		case 0x10 : // BPL
			switch(cycle++) {
				case 1: PC++;
						if ((ST&0x80))
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0x30 : // BMI
			switch(cycle++) {
				case 1: PC++;
						if (!(ST&0x80))
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0x50 : // BVC
			switch(cycle++) {
				// This has to be more generic because the drive byte ready
				// line is connected to the V flag pin on the disk drive CPU
				case 1: PC++;
						if (CheckVFlag())
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0x70 : // BVS
			switch(cycle++) {
				// This has to be more generic because the drive byte ready
				// line is connected to the V flag pin on the disk drive CPU
				case 1: PC++;
						if (!(CheckVFlag()))
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0x90 : // BCC
			switch(cycle++) {
				case 1: PC++;
						if (ST&0x01)
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0xB0 : // BCS
			switch(cycle++) {
				case 1: PC++;
						if (!(ST&0x01))
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0xD0 : // BNE
			switch(cycle++) {
				case 1: PC++;
						if (ST&0x02)
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		case 0xF0 : // BEQ
			switch(cycle++) {
				case 1: PC++;
						if (!(ST&0x02))
							cycle=0;
						break;
				case 2: if (!(((PC&0xFF)+(signed char) nextins)&0xFF00)) {
							PC+=(signed char) nextins;
							cycle=0;
						};
						break;
				case 3: PC+=(signed char) nextins;
						cycle=0;
						break;
			};
			break;

		// manipulating index registers

		case 0x88 : // DEY
			--Y;
			SETFLAGS_ZN(Y);
			cycle=0;
			break;

		case 0xC8 : // INY
			++Y;
			SETFLAGS_ZN(Y);
			cycle=0;
			break;

		case 0xCA : // DEX
			--X;
			SETFLAGS_ZN(X);
			cycle=0;
			break;

		case 0xE8 : // INX
			++X;
			SETFLAGS_ZN(X);
			cycle=0;
			break;

		case 0x00 : // BRK
			switch (cycle++) {
				case 1: if (!irq_sequence)
							PC++;
						break;
				case 2: push(PC>>8);
						break;
				case 3: push(PC&0xFF);
						break;
				case 4: // BRK pushes the status register to the stack with the B bit set
						// IRQ can still occur on BRK before saving the flags
						// if (!(ST&0x04)) irq_sequence = 0x10;
						push((ST|0x30)&~irq_sequence);
						// and then masks the interrupt bit
						ST|=0x04;
						break;
				case 5: break;
				case 6: PC=mem->Read(0xFFFE)|(mem->Read(0xFFFF)<<8);
						irq_sequence = 0x0;
						cycle=0;
						break;
			};
			break;

		/*case 0x00 : // BRK
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: push(PC>>8);
						break;
				case 3: push(PC&0xFF);
						break;
				case 4: // BRK pushes the status register to the stack with the B bit set
						// IRQ can still occur on BRK before saving the flags
						// if (!(ST&0x04)) irq_sequence = 0x10;
						push((ST|0x30));
						// and then masks the interrupt bit
						ST|=0x04;
						break;
				case 5: break;
				case 6: PC=mem->Read(0xFFFE)|(mem->Read(0xFFFF)<<8);
						cycle=0;
						break;
			};
			break;

		case 0x100 : // IRQ - pseudo opcode
			switch (cycle++) {
				case 1: break;
				case 2: push(PC>>8);
						break;
				case 3: push(PC&0xFF);
						break;
				case 4: push((ST|0x20)&0xEF);
						// and then masks the interrupt bit
						ST|=0x04;
						break;
				case 5: break;
				case 6: PC=mem->Read(0xFFFE)|(mem->Read(0xFFFF)<<8);
						//irq_sequence = 0x0;
						cycle=0;
						break;
			};
			break;*/

		case 0x40 : // RTI
			switch (cycle++) {
				case 1: //PC++;
						break;
				case 2: //PC++;
						SP++;
						break;
				case 3: // the B flag is already always cleared
						ST=pull();//&0xEF;
						SP++;
						break;
				case 4: PC=pull();
						SP++;
						break;
				case 5: PC|=pull()<<8;
						//ST&=0xFB;
						cycle=0;
						break;
			};
			break;

		case 0x60 : // RTS
			switch (cycle++) {
				case 1: //PC++;
						break;
				case 2: //PC++;
						SP++;
						break;
				case 3: PC=pull();
						SP++;
						break;
				case 4: PC|=pull()<<8;
						break;
				case 5: PC++;
						cycle=0;
						break;
			};
			break;

		// stack operations

		case 0x08 : // PHP
			if (cycle++==2) {
				// the PHP always pushes the status with the B flag set... 
				//ST = (ST&0xBF)|CheckVFlag();
				push(ST|0x30);
				cycle=0;
			};
			break;

		case 0x28 : // PLP
			if (cycle++==3) {
				SP++;
				ST=(pull() /*&0xEF*/);//|(0x20);
				cycle=0;
			};
			break;

		case 0x48 : // PHA
			if (cycle++==2) {
				push(AC);
				cycle=0;
			};
			break;

		case 0x68 : // PLA
			if (cycle++==3) {
				SP++;
				AC=pull();
				SETFLAGS_ZN(AC);
				cycle=0;
			};
			break;

		// inter-register operations

		case 0x8A : // TXA
			AC=X;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0xAA : // TAX
			X=AC;
			SETFLAGS_ZN(X);
			cycle=0;
			break;

		case 0x98 : // TYA
			AC=Y;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0xA8 : // TAY
			Y=AC;
			SETFLAGS_ZN(Y);
			cycle=0;
			break;

		case 0x9A : // TXS
			SP=X;
			cycle=0;
			break;

		case 0xBA : // TSX
			X=SP;
			SETFLAGS_ZN(X);
			cycle=0;
			break;

		// subroutine & unconditional jump

		case 0x20 : // JSR
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: // unknown
					break;
				case 3: push(PC>>8);
					break;
				case 4: push(PC&0xFF);
					break;
				case 5: PC=ptr=nextins|(mem->Read(PC)<<8);
						cycle=0;
						break;
			};
			break;

		case 0x4C : // JMP $0000
			if (cycle++==2) {
				PC=nextins|(mem->Read(PC+1)<<8);
				cycle=0;
			};
			break;

		case 0x6C : // JMP ($0000)
			if (cycle++==4) {
				ptr=nextins|(mem->Read(PC+1)<<8);
				// instruction does not handle page crossing
				PC=mem->Read(ptr)|(mem->Read( ptr&0xFF00 | (ptr+1)&0xFF) << 8);
				cycle=0;
			};
			break;

 		// Accumulator operations with immediate addressing

		case 0x09 : // ORA #$00
			PC++;
			AC|=nextins;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x29 : // AND #$00
			PC++;
			AC&=nextins;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x49 : // EOR #$00
			PC++;
			AC^=nextins;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x69 : // ADC #$00
			PC++;
			ADC(nextins);
			cycle=0;
			break;

		case 0xC9 : // CMP #$00
			PC++;
			DoCompare(AC,nextins);
			cycle=0;
			break;

		case 0xE9 : // SBC #$00
			PC++;
			SBC(nextins);
			cycle=0;
			break;

		// rotations

		case 0x0A : // ASL
			AC&0x80 ? ST|=0x01 : ST&=0xFE; // the Carry flag
			AC<<=1;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x2A : // ROL
			nextins=(AC<<1)|(ST&0x01);
			AC&0x80 ? ST|=0x01 : ST&=0xFE; // the Carry flag
			AC=nextins;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x4A : // LSR
			AC&0x01 ? ST|=0x01 : ST&=0xFE; // the Carry flag
			AC=AC>>1;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x6A : // ROR
			nextins=(AC>>1)|((ST&0x01)<<7);
			AC&0x01 ? ST|=0x01 : ST&=0xFE; // the Carry flag
			AC=nextins;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		// loads

		case 0xA0 : // LDY
			PC++;
			Y=nextins;
			SETFLAGS_ZN(Y);
			cycle=0;
			break;

		case 0xA2 : // LDX
			PC++;
			X=nextins;
			SETFLAGS_ZN(X);
			cycle=0;
			break;

		case 0xA9 : // LDA
			PC++;
			AC=nextins;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		// comparisons with immediate addressing

		case 0xC0 : // CPY
			PC++;
			DoCompare(Y,nextins);
			cycle=0;
			break;

		case 0xE0 : // CPX
			PC++;
			DoCompare(X,nextins);
			cycle=0;
			break;

		// BIT tests with accumulator

		case 0x24 : // BIT $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins=mem->Read(nextins);
						ST = (ST&0x3D) 
						| (nextins&0xC0) 
						| (((AC&nextins)==0)<<1);
						/*if (!(ST&0x40))
							ClearVFlag();*/
						cycle=0;
						break;
			};
			break;

		case 0x2C : // BIT $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						ST = (ST&0x3D) 
						| (nextins&0xC0) 
						| (((AC&nextins)==0)<<1);
						/*if (!(ST&0x40))
							ClearVFlag();*/
						cycle=0;
						break;
			};
			break;

		// Read instructions with absolute addressing

		case 0x0D : // ORA $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: AC|=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x2D : // AND $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: AC&=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
			};
			break;

		case 0x4D : // EOR $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: AC^=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x6D : // ADC $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3:	ADC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0x99 : // STA $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3:	break;
				case 4: mem->Write(ptr+Y,AC);
						cycle=0;
						break;
			};
			break;

		case 0xAC : // LDY $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: Y=mem->Read(ptr);
						SETFLAGS_ZN(Y);
						cycle=0;
						break;
			};
			break;

		case 0xCC : // CPY $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: DoCompare(Y,mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xEC : // CPX $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: DoCompare(X,mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xAD : // LDA $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: AC=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xCD : // CMP $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: DoCompare(AC,mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xED : // SBC $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: SBC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0x0E : // ASL $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4:	mem->Write(ptr,nextins);
						(nextins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						nextins<<=1;
						break;
				case 5:	mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x1E : // ASL $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: ptr+=X;
						break;
				case 4:	nextins=mem->Read(ptr);
						break;
				case 5:	mem->Write(ptr,nextins);
						(nextins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						nextins<<=1;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x2E : // ROL $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: farins=mem->Read(ptr);
						nextins=(farins<<1)|(ST&0x01);
						break;
				case 4:	mem->Write(ptr,farins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5:	mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x3E : // ROL $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: ptr+=X;
						break;
				case 4:	farins=mem->Read(ptr);
						nextins=(farins<<1)|(ST&0x01);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5:	mem->Write(ptr,farins);
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x4E : // LSR $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						(nextins&0x01) ? ST|=0x01 : ST&=0xFE;; // the Carry flag
						break;
				case 4:	mem->Write(ptr,nextins);
						nextins=nextins>>1;
						break;
				case 5:	mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x5E : // LSR $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: ptr+=X;
						break;
				case 4:	nextins=mem->Read(ptr);
						(nextins&0x01) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5:	mem->Write(ptr,nextins);
						nextins>>=1;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x6E : // ROR $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: farins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,farins);
						nextins=(farins>>1)|((ST&0x01)<<7);
						(farins&0x01) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x7E : // ROR $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: ptr+=X;
						break;
				case 4: farins=mem->Read(ptr);
						nextins=(farins>>1)|((ST&0x01)<<7);
						(farins&0x01) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5: mem->Write(ptr,farins);
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xAE : // LDX $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: X=mem->Read(ptr);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xCE : // DEC $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						--nextins;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xDE : // DEC $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: ptr+=X;
						break;
				case 4: nextins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,nextins);
						--nextins;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xEE : // INC $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						++nextins;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xFE : // INC $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: ptr+=X;
						break;
				case 4: nextins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,nextins);
						++nextins;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		// zero page indexed with X or Y

		case 0x94 : // STY $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: mem->Write(nextins,Y);
						cycle=0;
						break;
			};
			break;

		case 0x95 : // STA $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: mem->Write(nextins,AC);
						cycle=0;
						break;
			};
			break;

		case 0x96 : // STX $00,Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=Y;
						break;
				case 3: mem->Write(nextins,X);
						cycle=0;
						break;
			};
			break;

		case 0xB4 : // LDY $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: Y=mem->Read(nextins);
						SETFLAGS_ZN(Y);
						cycle=0;
						break;
			};
			break;

		case 0xB5 : // LDA $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: AC=mem->Read(nextins);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xB6 : // LDX $00,Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=Y;
						break;
				case 3: X=mem->Read(nextins);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xD5 : // CMP $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: DoCompare(AC,mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0x15 : // ORA $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: AC|=(mem->Read(nextins));
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x35 : // AND $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: AC&=(mem->Read(nextins));
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x16 : // ASL $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: farins=mem->Read(nextins);
						break;
				case 4: mem->Write(nextins,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x36 : // ROL $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						ptr=nextins;
						break;
				case 3: farins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,farins);
						nextins=(farins<<1)|((ST&0x01));
						farins&0x80 ? ST|=0x01 : ST&=0xFE;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		// absolute addressing indexed with X or Y

		case 0x19 : // ORA $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							AC|=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						}
						break;
				case 4: AC|=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x39 : // AND $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							AC&=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						}
						break;
				case 4: AC&=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x59 : // EOR $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							AC^=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						}
						break;
				case 4: AC^=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x79 : // ADC $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							ADC(mem->Read(ptr));
							cycle=0;
						}
						break;
				case 4: ADC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xB9 : // LDA $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							AC=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						}
						break;
				case 4: AC=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x1D : // ORA $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							AC|=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						};
						break;
				case 4: AC|=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x3D : // AND $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							AC&=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						};
						break;
				case 4: AC&=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x5D : // EOR $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							AC^=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						};
						break;
				case 4: AC^=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x7D : // ADC $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							ADC(mem->Read(ptr));
							cycle=0;
						};
						break;
				case 4: ADC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xBC : // LDY $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							Y=mem->Read(ptr);
							SETFLAGS_ZN(Y);
							cycle=0;
						};
						break;
				case 4: Y=mem->Read(ptr);
						SETFLAGS_ZN(Y);
						cycle=0;
						break;
			};
			break;

		case 0xBD : // LDA $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							AC=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						};
						break;
				case 4: AC=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xBE : // LDX $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							X=mem->Read(ptr);
							SETFLAGS_ZN(X);
							cycle=0;
						};
						break;
				case 4: X=mem->Read(ptr);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xD9 : // CMP $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: if (nextins+Y<0x100){
							DoCompare(AC,mem->Read(ptr+Y));
							cycle=0;
						};
						break;
				case 4: DoCompare(AC,mem->Read(ptr+Y));
						cycle=0;
						break;
			};
			break;

		case 0xF9 : // SBC $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100){
							SBC(mem->Read(ptr));
							cycle=0;
						};
						break;
				case 4: SBC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xDD : // CMP $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: if (nextins+X<0x100) {
							DoCompare(AC,mem->Read(ptr+X));
							cycle=0;
						};
						break;
				case 4: DoCompare(AC,mem->Read(ptr+X));
						cycle=0;
						break;
			};
			break;

		case 0xFD : // SBC $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: if (nextins+X<0x100) {
							SBC(mem->Read(ptr));
							cycle=0;
						};
						break;
				case 4: SBC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		// zero page operations

		case 0xA4 : // LDY $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: Y=mem->Read(nextins);
						SETFLAGS_ZN(Y);
						cycle=0;
						break;
			};
			break;

		case 0xC4 : // CPY $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: DoCompare(Y,mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0x05 : // ORA $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2:	AC|=mem->Read(nextins);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x55 : // EOR $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: AC^=mem->Read(nextins);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x65 : // ADC $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ADC(mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0x75 : // ADC $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ADC(mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0xA5 : // LDA $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: AC=mem->Read(nextins);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xC5 : // CMP $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: DoCompare(AC,mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0xE5 : // SBC $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: SBC(mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0xF5 : // SBC $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: SBC(mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0x06 : // ASL $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: farins=mem->Read(nextins);
						farins&0x80 ? ST|=0x01 : ST&=0xFE;
						break;
				case 3: mem->Write(nextins,farins);
						farins<<=1;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x26 : // ROL $00
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: farins=mem->Read(ptr);
						nextins=(farins<<1)|(ST&0x01);
						break;
				case 3: mem->Write(ptr,farins);
						farins&0x80 ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 4: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x25 : // AND $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: AC&=mem->Read(nextins);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x45 : // EOR $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: AC^=mem->Read(nextins);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x46 : // LSR $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: farins=mem->Read(nextins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 3: mem->Write(nextins,farins);
						farins>>=1;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x56 : // LSR $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: farins=mem->Read(nextins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 4: mem->Write(nextins,farins);
						farins>>=1;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x66 : // ROR $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=nextins;
						farins=mem->Read(ptr);
						nextins=(farins>>1)|((ST&0x01)<<7);
						break;
				case 3: mem->Write(ptr,farins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 4: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x76 : // ROR $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						ptr=nextins;
						break;
				case 3: farins=mem->Read(ptr);
						nextins=(farins>>1)|((ST&0x01)<<7);
						break;
				case 4: mem->Write(ptr,farins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xA6 : // LDX $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: X=mem->Read(nextins);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xC6 : // DEC $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: farins=mem->Read(nextins);
						break;
				case 3: mem->Write(nextins,farins);
						--farins;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xE4 : // CPX $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: DoCompare(X,mem->Read(nextins));
						cycle=0;
						break;
			};
			break;

		case 0xE6 : // INC $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: farins=mem->Read(nextins);
						break;
				case 3: mem->Write(nextins,farins);
						++farins;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xD6 : // DEC $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: farins=mem->Read(nextins);
						break;
				case 4: mem->Write(nextins,farins);
						--farins;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xF6 : // INC $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: farins=mem->Read(nextins);
						break;
				case 4: mem->Write(nextins,farins);
						++farins;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		// indexed indirect

		case 0x01 : // ORA ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: AC|=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x21 : // AND ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: AC&=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x41 : // EOR ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: AC^=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x61 : // ADC ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: ADC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0x81 : // STA ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: mem->Write(ptr,AC);
						cycle=0;
						break;
			};
			break;

		case 0xA1 : // LDA ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: AC=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xC1 : // CMP ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: DoCompare(AC,mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		case 0xE1 : // SBC ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins++);
						break;
				case 4: ptr|=(mem->Read(nextins)<<8);
						break;
				case 5: SBC(mem->Read(ptr));
						cycle=0;
						break;
			};
			break;

		// indirect indexed

		case 0x11 : // ORA ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							AC|=mem->Read(ptr+Y);
							cycle=0;
							SETFLAGS_ZN(AC);
						}
						break;
				case 5: AC|=mem->Read(ptr+Y);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x31 : // AND ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							AC&=mem->Read(ptr+Y);
							cycle=0;
							SETFLAGS_ZN(AC);
						}
						break;
				case 5: AC&=mem->Read(ptr+Y);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x51 : // EOR ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							AC^=mem->Read(ptr+Y);
							cycle=0;
							SETFLAGS_ZN(AC);
						}
						break;
				case 5: AC^=mem->Read(ptr+Y);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x71 : // ADC ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							ADC(mem->Read(ptr+Y));
							cycle=0;
						};
						break;
				case 5: ADC(mem->Read(ptr+Y));
						cycle=0;
						break;
			};
			break;

		case 0x91 : // STA ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: mem->Write(ptr+Y,AC);
						cycle=0;
						break;
			};
			break;

		case 0xB1 : // LDA ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							AC=mem->Read(ptr+Y);
							SETFLAGS_ZN(AC);
							cycle=0;
						}
						break;
				case 5: AC=mem->Read(ptr+Y);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xD1 : // CMP ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							DoCompare(AC,mem->Read(ptr+Y));
							cycle=0;
						}
						break;
				case 5: DoCompare(AC,mem->Read(ptr+Y));
						cycle=0;
						break;
			};
			break;

		case 0xF1 : // SBC ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							SBC(mem->Read(ptr+Y));
							cycle=0;
						};
						break;
				case 5: SBC(mem->Read(ptr+Y));
						cycle=0;
						break;
			};
			break;

		// storage functions

		case 0x84 : // STY $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: mem->Write(nextins,Y);
						cycle=0;
						break;
			};
			break;

		case 0x85 : // STA $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: mem->Write(nextins,AC);
						cycle=0;
						break;
			};
			break;

		case 0x86 : // STX $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: mem->Write(nextins,X);
						cycle=0;
						break;
			};
			break;

		case 0x8C : // STY $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: mem->Write(ptr,Y);
						cycle=0;
						break;
			};
			break;

		case 0x8D : // STA $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: mem->Write(ptr,AC);
						cycle=0;
						break;
			};
			break;

		case 0x8E : // STX $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: mem->Write(ptr,X);
						cycle=0;
						break;
			};
			break;

		case 0x9D : // STA $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: break;
				case 4: mem->Write(ptr+X,AC);
						cycle=0;
						break;
			};
			break;

		//----------------
		// illegal opcodes
		//----------------

		case 0x03 : // ASO/SLO ($00,X) : A <- (M << 1) \/ A
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=(mem->Read(nextins+1)<<8);
						break;
				case 5: farins=mem->Read(ptr);
						break;
				case 6: mem->Write(ptr,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						break;
				case 7: mem->Write(ptr,farins);
						AC|=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x07 : // ASO/SLO $00		: A <- (M << 1) \/ A
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: nextins=mem->Read(ptr);
						break;
				case 3: mem->Write(ptr,nextins);
						(nextins)&0x80 ? ST|=0x01 : ST&=0xFE;
						nextins<<=1;
						break;
				case 4: mem->Write(ptr,nextins);
						AC|=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x0B : // ANN/ANC #$00
		case 0x2B : // ANN/ANC #$00
			PC++;
			AC&=nextins;
			(AC&0x80) ? ST|=0x01 : ST&=0xFE;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x0C : // NOP $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0x0F : // ASO/SLO $0000		: A <- (M << 1) \/ A
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						(nextins)&0x80 ? ST|=0x01 : ST&=0xFE;
						nextins<<=1;
						break;
				case 5: mem->Write(ptr,nextins);
						AC|=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x13 : // ASO/SLO ($00),Y		: A <- (M << 1) \/ A
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=(mem->Read(nextins+1)<<8);
						break;
				case 4: break;
				case 5: farins=mem->Read(ptr+Y);
						break;
				case 6: mem->Write(ptr+Y,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						break;
				case 7: mem->Write(ptr+Y,farins);
						AC|=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x14 : // NOP $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0x17 : // ASO/SLO $00,X		: A <- (M << 1) \/ A
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=(nextins+X)&0xFF;
						break;
				case 3: farins = mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						break;
				case 5: mem->Write(ptr,farins);
						AC|=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x80 : // NOP #$00
		case 0x82 : // NOP #$00
		case 0x89 : // NOP #$00
		case 0xC2 : // NOP #$00
		case 0xE2 : // NOP #$00
			PC++;
		case 0x1A : // NOP
		case 0x3A : // NOP
		case 0x5A : // NOP
		case 0x7A : // NOP
		case 0xDA : // NOP
		case 0xFA : // NOP
			cycle=0;
			break;

		case 0x1B : // ASO/SLO $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: break;
				case 4:	farins=mem->Read(ptr);
						break;
				case 5:	mem->Write(ptr,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						break;
				case 6: mem->Write(ptr,farins);
						AC|=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x1C : // NOP $0000,X
		case 0x3C : // NOP $0000,X
		case 0x5C : // NOP $0000,X
		case 0x7C : // NOP $0000,X
		case 0xDC : // NOP $0000,X
		case 0xFC : // NOP $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: if (nextins+X<0x100)
							cycle=0;
						break;
				case 4: cycle=0;
						break;
			};
			break;

		case 0x1F : // ASO/SLO $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						ptr+=X;
						break;
				case 3: break;
				case 4:	farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						break;
				case 6: mem->Write(ptr,farins);
						AC|=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x23 : // RAN/RLA ($00,X) - ROL memory, AND accu, result into acc
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=(mem->Read(nextins+1)<<8);
						farins=mem->Read(ptr);
						break;
				case 5: nextins=(farins<<1)|(ST&0x01);
						mem->Write(ptr,nextins);
						break;
				case 6:	(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 7: AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x27 : // RAN/RLA $00 -		A <- (M << 1) /\ (A)
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: farins=mem->Read(ptr);
						break;
				case 3: mem->Write(ptr,farins);
						nextins=(farins<<1)|(ST&0x01);
						break;
				case 4: mem->Write(ptr,nextins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x2F : // RAN/RLA $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: farins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr, farins);
						nextins=(farins<<1)|(ST&0x01);
						break;
				case 5: mem->Write(ptr,nextins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x33 : // RAN/RLA ($00),Y -	A <- (M << 1) /\ (A) - not 100%
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: farins=mem->Read(ptr+Y);
						nextins=(farins<<1)|(ST&0x01);
						break;
				case 6: mem->Write(ptr+Y,farins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 7: mem->Write(ptr+Y,nextins);
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x34 : // NOP $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0x37 : // RAN/RLA $00,X -			A <- (M << 1) /\ (A)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						ptr=nextins;
						break;
				case 3: farins=mem->Read(ptr);
						nextins=(farins<<1)|(ST&0x01);
						break;
				case 4: mem->Write(ptr,farins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5: mem->Write(ptr,nextins);
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x3B : // RAN/RLA $0000,Y -	A <- (M << 1) /\ (A)
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: break;
				case 4:	farins=mem->Read(ptr+Y);
						break;
				case 5:	mem->Write(ptr+Y,farins);
						nextins=(farins<<1)|(ST&0x01);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 6: mem->Write(ptr+Y,nextins);
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x3F : // RAN/RLA $0000,X 
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						nextins=(farins<<1)|(ST&0x01);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5: mem->Write(ptr,farins);
						break;
				case 6: mem->Write(ptr,nextins);
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x43 : // LSE/SRE ($00,X) -	A <- (M >> 1) ^ A
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=(mem->Read(nextins+1)<<8);
						break;
				case 5: nextins=mem->Read(ptr)>>1;
						break;
				case 6: mem->Write(ptr,nextins);
						break;
				case 7: AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x47 : // LSE/SRE $00 -		A <- (M >> 1) \-/ A
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: nextins=mem->Read(ptr);
						break;
				case 3: mem->Write(ptr,nextins);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						nextins>>=1;
						break;
				case 4: mem->Write(ptr,nextins);
						AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;			

		case 0x4B : // ANL/ASR #$00
			PC++;
			AC=(AC&nextins);
			(AC&0x01) ? ST|=0x01 : ST&=0xFE;
			AC>>=1;
			SETFLAGS_ZN(AC);
			cycle=0;
			break;

		case 0x4F : // LSE/SRE $0000 - A <- (M >> 1) \-/ A
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						nextins>>=1;
						break;
				case 5: mem->Write(ptr,nextins);
						AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x53 : // LSE/SRE ($00),Y -	A <- (M >> 1) \-/ A	 - not 100%
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: nextins=mem->Read(ptr+Y);
						break;
				case 6: mem->Write(ptr+Y,nextins);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						nextins>>=1;
						break;
				case 7: mem->Write(ptr+Y,nextins);
						AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x54 : // NOP $00,X
			switch (cycle++) {
				case 1: break;
				case 2: PC++;
						break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0x57 : // LSE/SRE $00,X -			A <- (M >> 1) \-/ A	 - not 100%
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=nextins;
						nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						nextins>>=1;
						break;
				case 5: mem->Write(ptr,nextins);
						AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x5B : // LSE/SRE $0000,Y -	A <- (M >> 1) \-/ A                    
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE;
						farins>>=1;
						break;
				case 6: mem->Write(ptr,farins);
						AC^=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			}
			break;

		case 0x5F : // LSE/SRE $0000,X -	A <- (M >> 1) \-/ A
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						ptr+=X;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE;
						farins>>=1;
						break;
				case 6: mem->Write(ptr,farins);
						AC^=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x63 : // RAD/RRA ($00,X) - ROR memory, ADC accu, result into accu
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=(mem->Read(nextins+1)<<8);
						break;
				case 5: nextins=mem->Read(ptr);
						farins=(nextins>>1)|((ST&0x01)<<7);
						break;
				case 6: nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 7: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x04 : // NOP $00
		case 0x44 : // NOP $00
		case 0x64 : // NOP $00
			if (cycle++==2) {
				PC++;
				cycle=0;
			};
			break;

		case 0x67 : // RAD/RRA $00
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: nextins=mem->Read(ptr);
						break;
				case 3: mem->Write(ptr,nextins);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 4: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		// this instruction's decimal mode is not good at all!!!
		case 0x6B : // ARR $00
			switch (cycle++) {
				case 1: PC++;
						AC&=nextins;
						break;
				case 2: AC=(AC>>1)&((ST&0x01)<<7);
						SETFLAGS_ZN(AC);
						(AC&0x40) ? ST|=0x01 : ST&=0xFE;
						(AC&0x40)^((AC&0x20)<<1) ? SetVFlag() : ClearVFlag();
						cycle=0;
						//add_to_log("ARR encountered. %i",ST&0x01);
						break;
			};
			break;

		case 0x6F : // RAD/RRA $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 5: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x73 : // RAD/RRA ($00),Y -	A <- (M >> 1) + (A) + C 
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: nextins=mem->Read(ptr+Y);
						break;
				case 6: mem->Write(ptr+Y,nextins);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 7: mem->Write(ptr+Y,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x74 : // NOP $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0x77 : // RAD/RRA $00,X -	A <- (M >> 1) + (A) + C  - not good yet!!!!!!
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						ptr=nextins;
						break;
				case 3: nextins=mem->Read(ptr);
						farins=(nextins>>1)|((ST&0x01)<<7);
						break;
				case 4: nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 5: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x7B : // RAD/RRA $0000,Y -	A <- (M >> 1) + (A) + C   - not good yet!!!!!!
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: break;
				case 4: nextins=mem->Read(ptr);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 5: mem->Write(ptr,nextins);
						break;
				case 6: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			}
			break;

		case 0x7F : // RAD/RRA $8000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: break;
				case 4: nextins=mem->Read(ptr);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						break;
				case 5: mem->Write(ptr, nextins);
						break;
				case 6: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			}
			break;

		case 0x83 : // AXX/SAX ($00,X) -	M <- (A) /\ (X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=(mem->Read(nextins+1)<<8);
						break;
				case 5: mem->Write(ptr,AC&X);
						cycle=0;
						break;
			};
			break;

		case 0x87 : // AAX/AXR/SAX $00 - M <- (A) /\ (X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: mem->Write(nextins,AC&X);
						cycle=0;
						break;
			};
			break;

		case 0x8B : // TAN/ANE/XAA $00 -	M <-[(A)/\$EE] \/ (X)/\(M)
			switch (cycle++) {
				case 1: PC++;
						//mem->Write(nextins,(AC&0xEE)|(X&mem->Read(nextins)));
						AC=X&nextins&(AC|0xEE);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x8F : // AAX/SAX $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: mem->Write(ptr,AC&X);
						cycle=0;
						break;
			};
			break;
/*
        SHA $93,$9F     Store (A & X & (ADDR_HI + 1))
        SHX $9E         Store (X & (ADDR_HI + 1))
        SHY $9C         Store (Y & (ADDR_HI + 1))
        SHS $9B         SHA and TXS, where X is replaced by (A & X).

                        Note: The value to be stored is copied also
                        to ADDR_HI if page boundary is crossed.
*/
		case 0x93 : // AXI/SHA ($00),Y		(A) /\ (X) /\ (PCH+1) vagy 0000,X ki tudja???
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: mem->Write(ptr+Y,AC&X&(nextins+1));//check this!
						cycle=0;
						break;
			};
			break;

		case 0x97 : // AXY/SAX $00,Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=Y;
						break;
				case 3: mem->Write(nextins,AC&X);
						cycle=0;
						break;
			};
			break;

		case 0x9B : // AXS/SHS $0000,Y	- X <- (A) /\ (X), S <- (X) _plus_  M <- (X) /\ (PCH+1)
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						ptr+=Y;
						break;
				case 3: SP=AC&X;
						break;
				case 4: //mem->Write(ptr,AC&X&mem->Read(ptr+1)+1);
						mem->Write(ptr+Y,SP&(nextins+1));
						cycle=0;
						break;
			};
			break;

		case 0x9C : // AYI/SHY $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: break;
				case 4: //mem->Write(ptr,AC&Y&(((ptr+X)>>8)+1));
						if (nextins+X<256) {
							mem->Write(ptr+X,Y&(nextins+1));
						}
						cycle=0;
						break;
			};
			break;

		case 0x9E : // SXI/SHX $0000,Y		(X) /\ (((PC+Y)>>8)+1) vagy mi???
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: break;
				case 4: //mem->Write(ptr,X&(((ptr+Y)>>8)+1));
						if (nextins+Y<256) {
							mem->Write(ptr+Y,X&(nextins+1));
						}
						cycle=0;
						break;
			};
			break;

		// huh...????
		case 0x9F : // AXI/SHA $0000,Y		(A) /\ (X) /\ (((PC+Y)>>8)+1) vagy mi???
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: break;
				case 4: mem->Write(ptr+Y,AC&X&(nextins+1));
						cycle=0;
						break;
			};
			break;

		case 0xA3 : // LDT/LAX ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 5: X=AC=mem->Read(ptr);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xA7 : // LDT/LAX $00
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=nextins;
						break;
				case 3: X=AC=mem->Read(ptr);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xAB : // ANX/LXA #$00
			switch (cycle++) {
				case 1: PC++;
						X=AC=(nextins&(AC|0xEE));
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xAF : // LDT/LAX $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: X=AC=mem->Read(ptr);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xB3 : // LDT/LAX ($00),Y
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: if ((ptr&0x00FF)+Y<0x100) {
							X=AC=mem->Read(ptr+Y);
							SETFLAGS_ZN(X);
							cycle=0;
						}
						break;
				case 5: X=AC=mem->Read(ptr+Y);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xB7 : // LDT/LAX $00,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: X=AC=mem->Read(nextins);
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xBB : // LAE/TSA Stack-Pointer AND with memory, TSX, TXA
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: PC++;
						ptr+=Y;
						break;
				case 3: if (nextins+Y<0x100) {
							SP&=mem->Read(ptr);
							AC=X=SP;
							SETFLAGS_ZN(X);
							cycle=0;
						}
						break;
				case 4: SP&=mem->Read(ptr);
						AC=X=SP;
						SETFLAGS_ZN(X);
						cycle=0;
						break;
			};
			break;

		case 0xBF : // LDT/LAX $0000,Y
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: if (nextins+Y<0x100) {
							AC=X=mem->Read(ptr);
							SETFLAGS_ZN(AC);
							cycle=0;
						}
						break;
				case 4: AC=X=mem->Read(ptr);
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0xC3 : // DEM/DCP ($00,X)
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 5: nextins=mem->Read(ptr)-1;
						break;
				case 6: break;
				case 7: mem->Write(ptr,nextins);
						(AC>=nextins) ? ST|=0x01: ST&=0xFE;
						farins=AC-nextins;
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xC7 : // DEM/DCP $00
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: nextins=mem->Read(ptr);
						break;
				case 3: mem->Write(ptr,nextins);
						nextins -= 1;
						break;
				case 4:	mem->Write(ptr,nextins);
						(AC>=nextins) ? ST|=0x01 : ST&=0xFE;
						nextins=AC-nextins;
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xCB : // AXS/XAS/SBX -	X <- (X)/\(A) - M
			PC++;
			((X&AC) >= nextins) ? ST|=0x01 : ST&=0xFE;
			X = (X&AC) - nextins;
			SETFLAGS_ZN(X);
			cycle=0;
			break;

		case 0xCF : // DEM/DCP $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: farins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,farins);
						farins -= 1;
						break;
				case 5: mem->Write(ptr,farins);
						(AC>=farins) ? ST|=0x01 : ST&=0xFE;
						farins=AC-farins;
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xD3 : // DEM/DCP ($00),Y - DEC memory, CMP memory
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: farins=mem->Read(ptr+Y);
						break;
				case 6: mem->Write(ptr+Y,farins);
						farins -= 1;
						break;
				case 7: mem->Write(ptr+Y,farins);
						DoCompare(AC,farins);	
						cycle=0;
						break;
			};
			break;

		case 0xD4 : // NOP $0000,X
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0xD7 : // DEM/DCP $00,X -	M <- (M)-1, (A-M) -> NZC
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						ptr=nextins;
						break;
				case 3: nextins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,nextins);
						nextins-=1;
						break;
				case 5: mem->Write(ptr,nextins);
						(AC>=nextins) ? ST|=0x01 : ST&=0xFE;
						nextins=AC-nextins;
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xDB : // DEM/DCP $0000,Y : M <- (M)-1, (A-M) -> NZC
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						farins -= 1;
						break;
				case 6: mem->Write(ptr,farins);
						(AC>=farins) ? ST|=0x01 : ST&=0xFE;
						farins=AC-farins;
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xDF : // DEM/DCP $0000,X : M <- (M)-1, (A-M) -> NZC
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=X;
						PC++;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						farins -= 1;
						break;
				case 6: mem->Write(ptr,farins);
						(AC>=farins) ? ST|=0x01 : ST&=0xFE;
						farins=AC-farins;
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xE3 : // INB/ISB ($00,X) -	M <- (M) + 1, A <- (A) - M - C
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: ptr=mem->Read(nextins);
						break;
				case 4: ptr|=(mem->Read(nextins+1)<<8);
						break;
				case 5: nextins=mem->Read(ptr);
						break;
				case 6: mem->Write(ptr,nextins);
						nextins += 1;
						break;
				case 7: mem->Write(ptr,nextins);
						SBC(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xE7 : // INB/ISB $00 -	M <- (M) + 1, A <- (A) - M - C
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins;
						break;
				case 2: nextins=mem->Read(ptr);
						break;
				case 3: mem->Write(ptr,nextins);
						nextins += 1;
						break;
				case 4: mem->Write(ptr,nextins);
						SBC(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xEB : // SBC #$00 - illegal version
			PC++;
			SBC(nextins);
			cycle=0;
			break;

		case 0xEF : // INB/ISB $0000
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						break;
				case 3: farins=mem->Read(ptr);
						break;
				case 4: mem->Write(ptr,farins);
						farins += 1;
						break;
				case 5: mem->Write(ptr,farins);
						SBC(farins);
						cycle=0;
						break;
			};
			break;

		case 0xF3 : // INB/ISB ($00),Y - increase and subtract from AC
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: ptr=mem->Read(nextins);
						break;
				case 3: ptr|=mem->Read(nextins+1)<<8;
						break;
				case 4: break;
				case 5: farins=mem->Read(ptr+Y);
						break;
				case 6: mem->Write(ptr+Y,farins);
						farins+=1;
						break;
				case 7: mem->Write(ptr+Y,farins);
						SBC(farins);
						cycle=0;
						break;
			};
			break;

		case 0xF4 : // NOP $60,X
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: break;
				case 3: cycle=0;
						break;
			};
			break;

		case 0xF7 : // INB/ISB $00,X -	M <- (M) + 1, A <- (A) - M - C
			switch (cycle++) {
				case 1: PC++;
						break;
				case 2: nextins+=X;
						break;
				case 3: farins=mem->Read(nextins);
						break;
				case 4: mem->Write(nextins,farins);
						farins += 1;
						break;
				case 5: mem->Write(nextins,farins);
						SBC(farins);
						cycle=0;
						break;
			};
			break;

		case 0xFB : // INB/ISB $0000,Y - increase and subtract from AC
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: ptr+=Y;
						PC++;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						farins += 1;
						break;
				case 6: mem->Write(ptr,farins);
						SBC(farins);
						cycle=0;
						break;
			};
			break;

		case 0xFF : // INB/ISB $0000,X - increase and subtract from AC
			switch (cycle++) {
				case 1: PC++;
						ptr=nextins|(mem->Read(PC)<<8);
						break;
				case 2: PC++;
						ptr+=X;
						break;
				case 3: break;
				case 4: farins=mem->Read(ptr);
						break;
				case 5: mem->Write(ptr,farins);
						farins += 1;
						break;
				case 6: mem->Write(ptr,farins);
						SBC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x02 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x12 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x22 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x32 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x42 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x52 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x62 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x72 : // ABS, JAM, KIL or whatever - jams the machine
		case 0x92 : // ABS, JAM, KIL or whatever - jams the machine
		case 0xB2 : // ABS, JAM, KIL or whatever - jams the machine
		case 0xD2 : // ABS, JAM, KIL or whatever - jams the machine
		case 0xF2 : // ABS, JAM, KIL or whatever - jams the machine
			cycle = 0;
			PC = (PC-1)&0xFFFF;
			bp_reached = bp_active = cpu_jammed = true;
			//printf("CPU jammed\n");
			//exit(-1);
			
			break;

		default : cycle=1; // can't happen!
	};
	/*if ((*irq_register)&0x80 && cycle && !irq_sequence)
		IRQcount = 1;*/
};

// in these cycles, only write operations are allowed for the CPU
void CPU::stopcycle()
{
	//unsigned int old_cycle = cycle;

	switch (currins) {

		case 0x00 : // BRK
			switch (cycle) {
				case 2: push(PC>>8);
						cycle++;
						break;
				case 3: push(PC&0xFF);
						cycle++;
						break;
				case 4: // BRK pushes the status register to the stack with the B bit set
						// if not an IRQ
						push((ST|0x30)&~irq_sequence);
						//push(ST&0xEF);
						// and then masks the interrupt bit
						ST|=0x04;
						cycle++;
						break;
			};
			break;

		/*case 0x100 : // IRQ
			switch (cycle) {
				case 2: push(PC>>8);
						cycle++;
						break;
				case 3: push(PC&0xFF);
						cycle++;
						break;
				case 4: push((ST|0x20)&0xEF);
						// and then masks the interrupt bit
						ST|=0x04;
						cycle++;
						break;
			}
			break;

		case 0x00 : // BRK
			switch (cycle) {
				case 2: push(PC>>8);
						cycle++;
						break;
				case 3: push(PC&0xFF);
						cycle++;
						break;
				case 4: // BRK pushes the status register to the stack with the B bit set
						// if not an IRQ
						push((ST|0x30));
						// and then masks the interrupt bit
						ST|=0x04;
						cycle++;
						break;
			};
			break;*/

		case 0x08 : // PHP
			if (cycle==2) {
				// the PHP always pushes the status with the B flag set... 
				push(ST|0x30);
				cycle=0;
			};
			break;

		case 0x48 : // PHA
			if (cycle==2) {
				push(AC);
				cycle=0;
			};
			break;

		case 0x20 : // JSR
			switch (cycle) {
				case 3: push(PC>>8);
					cycle++;
					break;
				case 4: push(PC&0xFF);
					cycle++;
					break;
			};
			break;

		case 0x06 : // ASL $00
			switch (cycle) {
				case 3: mem->Write(nextins,farins);
						farins<<=1;
						++cycle;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x07 : // ASO/SLO $00		: A <- (M << 1) \/ A
			switch (cycle) {
				case 3: mem->Write(ptr,nextins);
						(nextins)&0x80 ? ST|=0x01 : ST&=0xFE;
						nextins<<=1;
						cycle++;
						break;
				case 4: mem->Write(ptr,nextins);
						AC|=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x0E : // ASL $0000
			switch (cycle) {
				case 4:	mem->Write(ptr,nextins);
						(nextins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						nextins<<=1;
						++cycle;
						break;
				case 5:	mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x0F : // ASO/SLO $0000		: A <- (M << 1) \/ A
			switch (cycle) {
				case 4: mem->Write(ptr,nextins);
						(nextins)&0x80 ? ST|=0x01 : ST&=0xFE;
						nextins<<=1;
						cycle++;
						break;
				case 5: mem->Write(ptr,nextins);
						AC|=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x16 : // ASL $00,X
			switch (cycle) {
				case 4: mem->Write(nextins,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						++cycle;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x17 : // ASO/SLO $00,X		: A <- (M << 1) \/ A
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						(farins)&0x80 ? ST|=0x01 : ST&=0xFE;
						farins<<=1;
						cycle++;
						break;
				case 5: mem->Write(ptr,farins);
						AC|=farins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x1E : // ASL $0000,X
			switch (cycle) {
				case 5:	mem->Write(ptr,nextins);
						(nextins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						nextins<<=1;
						++cycle;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x26 : // ROL $00
			switch (cycle) {
				case 3: mem->Write(ptr,farins);
						farins&0x80 ? ST|=0x01 : ST&=0xFE; // the Carry flag
						++cycle;
						break;
				case 4: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x27 : // RAN/RLA $00 -		A <- (M << 1) /\ (A)
			switch (cycle) {
				case 3: mem->Write(ptr,farins);
						nextins=(farins<<1)|(ST&0x01);
						cycle++;
						break;
				case 4: mem->Write(ptr,nextins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x2E : // ROL $0000
			switch (cycle) {
				case 4:	mem->Write(ptr,farins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						++cycle;
						break;
				case 5:	mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x2F : // RAN/RLA $0000
			switch (cycle) {
				case 4: mem->Write(ptr, farins);
						nextins=(farins<<1)|(ST&0x01);
						cycle++;
						break;
				case 5: mem->Write(ptr,nextins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x36 : // ROL $00,X
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						nextins=(farins<<1)|((ST&0x01));
						farins&0x80 ? ST|=0x01 : ST&=0xFE;
						++cycle;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x37 : // RAN/RLA $00,X -			A <- (M << 1) /\ (A)
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						(farins&0x80) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						cycle++;
						break;
				case 5: mem->Write(ptr,nextins);
						AC&=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x3E : // ROL $0000,X
			switch (cycle) {
				case 5:	mem->Write(ptr,farins);
						++cycle;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x46 : // LSR $00
			switch (cycle) {
				case 3: mem->Write(nextins,farins);
						farins>>=1;
						++cycle;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x47 : // LSE/SRE $00 -		A <- (M >> 1) \-/ A
			switch (cycle) {
				case 3: mem->Write(ptr,nextins);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						nextins>>=1;
						cycle++;
						break;
				case 4: mem->Write(ptr,nextins);
						AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;	

		case 0x4E : // LSR $0000
			switch (cycle) {
				case 4:	mem->Write(ptr,nextins);
						nextins=nextins>>1;
						++cycle;
						break;
				case 5:	mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x4F : // LSE/SRE $0000 - A <- (M >> 1) \-/ A
			switch (cycle) {
				case 4: mem->Write(ptr,nextins);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						nextins>>=1;
						cycle++;
						break;
				case 5: mem->Write(ptr,nextins);
						AC^=nextins;
						SETFLAGS_ZN(AC);
						cycle=0;
						break;
			};
			break;

		case 0x56 : // LSR $00,X
			switch (cycle) {
				case 4: mem->Write(nextins,farins);
						farins>>=1;
						++cycle;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0x5E : // LSR $0000,X
			switch (cycle) {
				case 5:	mem->Write(ptr,nextins);
						nextins>>=1;
						++cycle;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x66 : // ROR $00
			switch (cycle) {
				case 3: mem->Write(ptr,farins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE; // the Carry flag
						++cycle;
						break;
				case 4: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x67 : // RAD/RRA $00
			switch (cycle) {
				case 3: mem->Write(ptr,nextins);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						cycle++;
						break;
				case 4: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x6E : // ROR $0000
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						nextins=(farins>>1)|((ST&0x01)<<7);
						(farins&0x01) ? ST|=0x01 : ST&=0xFE; // the Carry flag
						++cycle;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x6F : // RAD/RRA $0000
			switch (cycle) {
				case 4: mem->Write(ptr,nextins);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						cycle++;
						break;
				case 5: mem->Write(ptr,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x73 : // RAD/RRA ($00),Y -	A <- (M >> 1) + (A) + C 
			switch (cycle) {
				case 6: mem->Write(ptr+Y,nextins);
						farins=(nextins>>1)|((ST&0x01)<<7);
						nextins&0x01 ? ST|=0x01 : ST&=0xFE;
						cycle++;
						break;
				case 7: mem->Write(ptr+Y,farins);
						ADC(farins);
						cycle=0;
						break;
			};
			break;

		case 0x76 : // ROR $00,X
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						farins&0x01 ? ST|=0x01 : ST&=0xFE; // the Carry flag
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0x7E : // ROR $0000,X
			switch (cycle) {
				case 5: mem->Write(ptr,farins);
						++cycle;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		// indexed indirect

		case 0x81 : // STA ($00,X)
			if (cycle==5) {
				mem->Write(ptr,AC);
				cycle=0;
			};
			break;

		case 0x83 : // AXX/SAX ($00,X) -	M <- (A) /\ (X)
			if (cycle==5) {
				mem->Write(ptr,nextins);
				cycle=0;
			};
			break;

		case 0x84 : // STY $00
			if (cycle==2) {
				mem->Write(nextins,Y);
				cycle=0;
			};
			break;

		case 0x85 : // STA $00
			if (cycle==2) {
				mem->Write(nextins,AC);
				cycle=0;
			};
			break;

		case 0x86 : // STX $00
			if (cycle==2) {
				mem->Write(nextins,X);
				cycle=0;
			};
			break;

		case 0x87 : // AAX/AXR/SAX $00 - M <- (A) /\ (X)
			if (cycle==2) {
				mem->Write(nextins,AC&X);
				cycle=0;
			};
			break;

		case 0x8C : // STY $0000
			if (cycle==3) {
				mem->Write(ptr,Y);
				cycle=0;
			};
			break;

		case 0x8D : // STA $0000
			if (cycle==3) {
				mem->Write(ptr,AC);
				cycle=0;
			};
			break;

		case 0x8E : // STX $0000
			if (cycle==3) {
				mem->Write(ptr,X);
				cycle=0;
			};
			break;

		case 0x8F : // AAX/SAX $0000
			if (cycle==3) {
				mem->Write(ptr,AC&X);
				cycle=0;
			};
			break;

		// indirect indexed

		case 0x91 : // STA ($00),Y
			if (cycle==5) {
				mem->Write(ptr+Y,AC);
				cycle=0;
			};
			break;

		// zero page indexed with X or Y

		case 0x94 : // STY $00,X
			switch (cycle) {
				case 3: mem->Write(nextins,Y);
						cycle=0;
						break;
			};
			break;

		case 0x95 : // STA $00,X
			switch (cycle) {
				case 3: mem->Write(nextins,AC);
						cycle=0;
						break;
			};
			break;

		case 0x96 : // STX $00,Y
			switch (cycle) {
				case 3: mem->Write(nextins,X);
						cycle=0;
						break;
			};
			break;

		case 0x97 : // AXY/SAX $00,Y
			if (cycle==3) {
				mem->Write(nextins,AC&X);
				cycle=0;
			};
			break;

		case 0x99 : // STA $0000,Y
			if (cycle==4) {
				mem->Write(ptr+Y,AC);
				cycle=0;
			};
			break;

		case 0x9C : // AYI/SHY $0000,X
			if (cycle==4) {
				if (nextins+X<256)
					mem->Write(ptr+X,Y&(nextins+1));
				cycle=0;
			};
			break;

		case 0x9D : // STA $0000,X
			if (cycle==4) {
				mem->Write(ptr+X,AC);
				cycle=0;
			};
			break;

		case 0xC6 : // DEC $00
			switch (cycle) {
				case 3: mem->Write(nextins,farins);
						--farins;
						++cycle;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xC7 : // DEM/DCP $00
			switch (cycle) {
				case 3: mem->Write(ptr,nextins);
						nextins -= 1;
						cycle++;
						break;
				case 4:	mem->Write(ptr,nextins);
						(AC>=nextins) ? ST|=0x01 : ST&=0xFE;
						nextins=AC-nextins;
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xCE : // DEC $0000
			switch (cycle) {
				case 4: mem->Write(ptr,nextins);
						--nextins;
						++cycle;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xCF : // DEM/DCP $0000
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						farins -= 1;
						cycle++;
						break;
				case 5: mem->Write(ptr,farins);
						(AC>=farins) ? ST|=0x01 : ST&=0xFE;
						farins=AC-farins;
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xD6 : // DEC $00,X
			switch (cycle) {
				case 4: mem->Write(nextins,farins);
						--farins;
						++cycle;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xD7 : // DEM/DCP $00,X -	M <- (M)-1, (A-M) -> NZC
			switch (cycle) {
				case 4: mem->Write(ptr,nextins);
						nextins-=1;
						cycle++;
						break;
				case 5: mem->Write(ptr,nextins);
						(AC>=nextins) ? ST|=0x01 : ST&=0xFE;
						nextins=AC-nextins;
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xDE : // DEC $0000,X
			switch (cycle) {
				case 5: mem->Write(ptr,nextins);
						--nextins;
						++cycle;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xE6 : // INC $00
			switch (cycle) {
				case 3: mem->Write(nextins,farins);
						++farins;
						++cycle;
						break;
				case 4: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xE7 : // INB/ISB $00 -	M <- (M) + 1, A <- (A) - M - C
			switch (cycle) {
				case 3: mem->Write(ptr,nextins);
						nextins += 1;
						cycle++;
						break;
				case 4: mem->Write(ptr,nextins);
						SBC(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xEE : // INC $0000
			switch (cycle) {
				case 4: mem->Write(ptr,nextins);
						++nextins;
						++cycle;
						break;
				case 5: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xEF : // INB/ISB $0000 - should be here but BAG protector won't work!
			switch (cycle) {
				case 4: mem->Write(ptr,farins);
						farins += 1;
						cycle++;
						break;
				case 5: mem->Write(ptr,farins);
						SBC(farins);
						cycle=0;
						break;
			};
			break;

		case 0xF6 : // INC $00,X
			switch (cycle) {
				case 4: mem->Write(nextins,farins);
						++farins;
						++cycle;
						break;
				case 5: mem->Write(nextins,farins);
						SETFLAGS_ZN(farins);
						cycle=0;
						break;
			};
			break;

		case 0xFE : // INC $0000,X
			switch (cycle) {
				case 5: mem->Write(ptr,nextins);
						++nextins;
						++cycle;
						break;
				case 6: mem->Write(ptr,nextins);
						SETFLAGS_ZN(nextins);
						cycle=0;
						break;
			};
			break;

		case 0xFF : // INB/ISB $0000,X - increase and subtract from AC
			switch (cycle) {
				case 5: mem->Write(ptr,farins);
						farins += 1;
						cycle++;
						break;
				case 6: mem->Write(ptr,farins);
						SBC(farins);
						cycle=0;
						break;
				default:
					return;
			};
			break;
	}
	if ((IRQcount || (!irq_sequence && (*irq_register)&0x80)) )
		IRQcount++;
}


/*
void CPU::_stopcycle()
{
	switch (currins) {

	case 0:
		if (cycle>1 && cycle<5)
			process();
		break;

	case 0x20:
		if (cycle==3 || cycle==4)
			process();
		break;

	default:
		switch (write_cycles[currins]) {
			case 2:
				break;
			case 1:
				break;
			default:
				break;
		}
		break;
	}
}*/

CPU::~CPU()
{
}


