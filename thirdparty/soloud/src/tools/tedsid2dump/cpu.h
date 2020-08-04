#ifndef _CPU_H
#define _CPU_H

#define SETFLAGS_ZN(VALUE) ST = (ST&0x7D)|(((VALUE)==0)<<1)|((VALUE)&0x80)

#include "mem.h"

class TED;

class CPU {
	protected:
		//unsigned int currins;
		unsigned char currins;
		unsigned char nextins;
		unsigned char farins;
		unsigned int  ptr;
		unsigned int  PC;
		unsigned char  SP;
						   // 76543210
		unsigned int  ST; // NV1BDIZC
		unsigned char  AC;
		unsigned char  X;
		unsigned char  Y;
		void ADC(const unsigned char value);
		void SBC(const unsigned char value);
		void DoCompare(unsigned char reg, unsigned char value);
		unsigned int cycle;
		unsigned int IRQcount;
		unsigned char *irq_register;
		unsigned char *stack;
		unsigned char irq_sequence;
		MemoryHandler *mem;
		virtual unsigned char CheckVFlag() { return (ST&0x40); };
		inline virtual void ClearVFlag() { ST&=0xBF; };
		inline void SetVFlag() { ST|=0x40; };

	public:
		CPU(MemoryHandler *memhandler, unsigned char *irqreg, unsigned char *cpustack);
		virtual ~CPU();
		void Reset(void);
		void softreset(void);
		void setPC(unsigned int addr);
		void setST(unsigned char st) { ST = st; };
		void process();
		void stopcycle();
		
		unsigned int getPC() { return PC; };
		unsigned int getSP() { return SP; };
		unsigned int getST() { return ST; };
		unsigned int getAC() { return AC; };
		unsigned int getX() { return X; };
		unsigned int getY() { return Y; };
		unsigned int getnextins() { return nextins; };
		unsigned int getptr() { return ptr; };
		unsigned int getcycle() { return cycle; };
		unsigned int getcins();

		bool saveshot(void *cpudump);
		bool loadshot(void *cpudump);
		// breakpoint variables
		static bool bp_active;
		static bool bp_reached;
		struct {
  			unsigned int address;
  			bool enabled;
  			bool slot_free;  			
		} bp[11];
		unsigned int nr_activebps;
		bool cpu_jammed;
		static const unsigned int nr_of_bps;	
};

inline void CPU::ADC(const unsigned char value)
{
	if (ST&0x08) {
		unsigned int bin_adc = AC + value + (ST&0x01);
		unsigned char AL, AH;

		AL=(AC&0x0F) + (value & 0x0F) + (ST&0x01);
		AH=(AC >> 4) + (value >> 4 ) ;
		// fix lower nybble
		if (AL>9) {
			AL+=6;
			AH++;
		}
		// zero bit depends on the normal ADC...
		(bin_adc)&0xFF ? ST&=0xFD : ST|=0x02;
		// negative flag also...
		( AH & 0x08 ) ? ST|=0x80 : ST&=0x7F; 
		// V flag
		((((AH << 4) ^ AC) & 0x80) && !((AC ^ value) & 0x80)) ? SetVFlag() : ClearVFlag();
		// fix upper nybble
		if (AH>9) 
			AH+=6;
		// set the Carry if the decimal add has an overflow
		(AH > 0x0f) ? ST|=0x01 : ST&=0xFE;
		// calculate new AC value
		AC = (AH<<4)|(AL & 0x0F);
	} else {
		unsigned int bin_adc = AC + value + (ST&0x01);
		(bin_adc>=0x100) ? ST|=0x01 : ST&=0xFE;
		(!((AC ^ value) & 0x80) && ((AC ^ bin_adc) & 0x80) ) ? SetVFlag() : ClearVFlag();
		AC=(unsigned char) bin_adc;
		SETFLAGS_ZN(AC);
	}
}

inline void CPU::SBC(const unsigned char value)
{
	if (ST&0x08) { // if in decimal mode
		unsigned int bin_sbc = (AC - value - (1-(ST&0x01)));
		unsigned int dec_sbc = (AC & 0x0F) - (value & 0x0F) - (1-(ST&0x01));
		// Calculate the upper nybble.
		// fix upper nybble
		if (dec_sbc&0x10)
			dec_sbc = ((dec_sbc-6)&0x0F) | ((AC&0xF0) - (value&0xF0) - 0x10);
		else
			dec_sbc = (dec_sbc&0x0F) | ((AC&0xF0) - (value&0xF0));
		
		if (dec_sbc&0x100)
			dec_sbc-= 0x60;

		// all flags depend on the normal SBC...
		(bin_sbc<0x100) ? ST|=0x01 : ST&=0xFE ; // carry flag
		SETFLAGS_ZN( bin_sbc & 0xFF );
		((AC^bin_sbc)&0x80 && (AC^value)&0x80 ) ? SetVFlag() : ClearVFlag(); // V flag

		AC=(unsigned char) dec_sbc;

	} else {
		unsigned int bin_sbc = (AC - value - (1-(ST&0x01)));
		(bin_sbc<0x100) ? ST|=0x01 : ST&=0xFE;
		(((AC ^ value) & 0x80) && ((AC ^ bin_sbc) & 0x80) ) ? SetVFlag() : ClearVFlag();
		AC=(unsigned char) bin_sbc;
		SETFLAGS_ZN(AC);
	}
}



#endif // _CPU_H
