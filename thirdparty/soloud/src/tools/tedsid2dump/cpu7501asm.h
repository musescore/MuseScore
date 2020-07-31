#pragma once

enum  { // AsmAdressingMode
	AM_UNKNOWN = 0,
	AM_IMPLIED,
	AM_IMMEDIATE,
	AM_ABSOLUTE,
	AM_INDIRECT,
	AM_ZP,
	AM_ZPIX,
	AM_ZPIY,
	AM_ABSOLUTEIX,
	AM_ABSOLUTEIY,
	AM_INDIRECTINDEXED,
	AM_INDEXEDINDIRECT,
	AM_RELATIVE
};

typedef struct INSTR
{
  char name[4];
  unsigned int  type;
  int  cycles;
  unsigned char ext;
  unsigned char iclass;
} instructions;

/*
   types :   1 - one-unsigned char instruction - eg.: nop
             2 - two-unsigned char instruction eg.: lda #$20
             3 - three-unsigned char instruction eg.: jmp $2000
             4 - three-unsigned char indirect instruction eg.: jmp ($2000)
             5 - two-unsigned char zero slap eg.: lda $50
             6 - two-unsigned char zero slap X indexed eg.: lda $60,x
             7 - two-unsigned char zero slap Y indexed eg.: ldx $70,y
             8 - three-unsigned char X indexed eg.: sta $8000,x
             9 - three-unsigned char Y indexed eg.: sta $8000,y
            10 - two-unsigned char zero slap Y, indexed eg.: lda ($a0),Y
            11 - two-unsigned char zero slap X, indexed eg.: lda ($b0,X)
            12 - two-unsigned char relative jump eg.: beq $c000

	  ext :   0 - regular assembler instruction mnemonic
    		1 - extended assembler instruction mnemonic
    		2 - not an assembly instruction

	class : 0 - special BRK,JAM,NOP : nothing affected
			1 - affects PC
			2 - affects A only
			3 - affects X only
			4 - affects Y only
			5 - affects memory

			9 - not dealt with
*/

const int typlen[13]={0,1,2,3,3,2,2,2,3,3,2,2,2};

/* 6510,7510 assembler instructions */
const instructions ins[256] =
						{ {"BRK", AM_IMPLIED,7,0,0},   // 00 - BRK
                          {"ORA",11,6,0,3},   // 01 - ORA ($B0,X)
                          {"ABS", AM_IMPLIED,2,1,0},   //*02 - ABS
                          {"SLO",11,8,1,9},   //*03 - SLO ($B0,X)
                          {"NOP", 5,3,1,0},   //*04 - NOP $50
                          {"ORA", 5,3,0,2},   // 05 - ORA $50
                          {"ASL", 5,5,0,5},   // 06 - ASL $50
                          {"SLO", 5,5,1,9},   //*07 - SLO $50
                          {"PHP", AM_IMPLIED,3,0,9},   // 08
                          {"ORA", AM_IMMEDIATE,2,0,2},   // 09 - ORA #$20
                          {"ASL", AM_IMPLIED,2,0,2},   // 0A - ASL
                          {"ANC", AM_IMMEDIATE,2,1,9},   //*0B - ANC #$20
                          {"NOP", 3,4,1,0},   //*0C - NOP $3000
                          {"ORA", 3,4,0,2},   // 0D - ORA $3000
                          {"ASL", 3,6,0,5},   // 0E - ASL $3000
                          {"SLO", 3,6,1,9},   //*0F - SLO $3000

                          {"BPL",12,4,0,9},   // 10 - BPL $C000 / add +1 if branch on same page, +2 othervise
                          {"ORA",10,6,0,2},   // 11 - ORA ($A0),Y
                          {"ABS", AM_IMPLIED,2,1,1},   //*12 - ABS
                          {"SLO",10,9,1,9},   //*13 - SLO ($A0),Y
                          {"NOP", 6,4,1,0},   //*14 - NOP $60,X
                          {"ORA", 6,4,0,2},   // 15 - ORA $60,X
                          {"ASL", 6,6,0,5},   // 16 - ASL $60,X
                          {"SLO", 6,6,1,9},   //*17 - SLO $60,X
                          {"CLC", AM_IMPLIED,2,0,9},   // 18 - CLC
                          {"ORA", 9,5,0,2},   // 19 - ORA $9000,Y / Add +1 on page crossing
                          {"NOP", AM_IMPLIED,2,1,0},   //*1A - NOP
                          {"SLO", 9,8,1,9},   //*1B - SLO $9000,Y
                          {"NOP", 8,5,1,0},   //*1C - NOP $8000,X
                          {"ORA", 8,5,0,2},   // 1D - ORA $8000,X / Add +1 on page crossing
                          {"ASL", 8,7,0,5},   // 1E - ASL $8000,X
                          {"SLO", 8,8,1,9},   //*1F - SLO $8000,X

                          {"JSR", 3,6,0,9},   // 20 - JSR $3000
                          {"AND",11,6,0,9},   // 21 - AND ($B0,X)
                          {"ABS", AM_IMPLIED,2,1,1},   //*22 - ABS
                          {"RLA",11,8,1,9},   //*23 - RLA ($B0,X)
                          {"BIT", 5,3,0,7},   // 24 - BIT $50
                          {"AND", 5,3,0,2},   // 25 - AND $50
                          {"ROL", 5,5,0,5},   // 26 - ROL $50
                          {"RLA", 5,5,1,9},   //*27 - RLA $50
                          {"PLP", AM_IMPLIED,4,0,9},   // 28 - PLP
                          {"AND", AM_IMMEDIATE,2,0,2},   // 29 - AND #$20
                          {"ROL", AM_IMPLIED,2,0,2},   // 2A - ROL
                          {"ANC", AM_IMMEDIATE,2,1,9},   //*2B - ANC #$20
                          {"BIT", 3,4,0,7},   // 2C - BIT $3000
                          {"AND", 3,4,0,2},   // 2D - AND $3000
                          {"ROL", 3,6,0,5},   // 2E - ROL $3000
                          {"RLA", 3,6,1,9},   //*2F - RLA $3000

                          {"BMI",12,4,0,9},   // 30 - BPL $C000 / add +1 if branch on same page, +2 othervise
                          {"AND",10,6,0,2},   // 31 - AND ($A0),Y
                          {"ABS", AM_IMPLIED,2,1,1},   //*32 - ABS
                          {"RLA",10,9,1,9},   //*33 - RLA ($A0),Y
                          {"NOP", 6,4,1,0},   //*34 - NOP $60,X
                          {"AND", 6,4,0,9},   // 35 - AND $60,X
                          {"ROL", 6,6,0,5},   // 36 - ROL $60,X
                          {"RLA", 6,6,1,9},   //*37 - RLA $60,X
                          {"SEC", AM_IMPLIED,2,0,9},   // 38 - SEC
                          {"AND", 9,5,0,9},   // 39 - AND $9000,Y / add +1 if page boundary is crossed
                          {"NOP", AM_IMPLIED,2,1,0},   //*3A - NOP
                          {"RLA", 9,8,1,9},   //*3B - RLA $9000,Y
                          {"NOP", 8,5,1,0},   //*3C - NOP $8000,X
                          {"AND", 8,5,0,9},   // 3D - AND $8000,X / add +1 if page boundary is crossed
                          {"ROL", 8,7,0,5},   // 3E - ROL $8000,X
                          {"RLA", 8,8,1,9},   //*3F - RLA $8000,X

                          {"RTI", AM_IMPLIED,6,0,9},   // 40 - RTI
                          {"EOR",11,6,0,2},   // 41 - EOR ($B0,X)
                          {"ABS", AM_IMPLIED,2,1,1},   //*42 - ABS
                          {"SRE",11,8,1,9},   //*43 - SRE ($B0,X)
                          {"NOP", 5,3,1,0},   //*44 - NOP $50
                          {"EOR", 5,3,0,2},   // 45 - EOR $50
                          {"LSR", 5,5,0,5},   // 46 - LSR $50
                          {"SRE", 5,5,1,9},   //*47 - SRE $50
                          {"PHA", AM_IMPLIED,3,0,9},   // 48 - PHA
                          {"EOR", AM_IMMEDIATE,2,0,2},   // 49 - EOR #$20
                          {"LSR", AM_IMPLIED,2,0,2},   // 4A - LSR
                          {"ANL", AM_IMMEDIATE,2,1,9},   //*4B - ANL #$20
                          {"JMP", 3,3,0,9},   // 4C - JMP $3000
                          {"EOR", 3,4,0,9},   // 4D - EOR $3000
                          {"LSR", 3,6,0,5},   // 4E - LSR $3000
                          {"SRE", 3,6,1,9},   //*4F - SRE $3000

                          {"BVC",12,4,0,9},   // 50 - BVC $C000 / add +1 if branch on same page, +2 othervise
                          {"EOR",10,6,0,9},   // 51 - EOR ($A0),Y / add +1 if page boundary is crossed
                          {"ABS", AM_IMPLIED,2,1,1},   //*52 - ABS
                          {"SRE",10,9,1,9},   //*53 - SRE ($A0),Y
                          {"NOP", 6,4,1,0},   //*54 - NOP $60,X
                          {"EOR", 6,4,0,9},   // 55 - EOR $60,X
                          {"LSR", 6,6,0,5},   // 56 - LSR $60,X
                          {"SRE", 6,6,1,9},   //*57 - SRE $60,X
                          {"CLI", AM_IMPLIED,2,0,9},   // 58 - CLI
                          {"EOR", 9,5,0,2},   // 59 - EOR $9000,Y / add +1 if page boundary is crossed
                          {"NOP", AM_IMPLIED,2,1,0},   //*5A - NOP
                          {"SRE", 9,8,1,9},   //*5B - SRE $9000,Y
                          {"NOP", 8,5,1,0},   //*5C - NOP $8000,X
                          {"EOR", 8,5,0,2},   // 5D - EOR $8000,X / add +1 if page boundary is crossed
                          {"LSR", 8,7,0,5},   // 5E - LSR $8000,X
                          {"SRE", 8,8,1,9},   //*5F - SRE $8000,X

                          {"RTS", AM_IMPLIED,6,0,9},   // 60 - RTS
                          {"ADC",11,6,0,2},   // 61 - ADC ($B0,X)
                          {"ABS", AM_IMPLIED,2,1,1},   //*62 - ABS
                          {"RRA",11,8,1,9},   //*63 - RRA ($B0,X)
                          {"NOP", 5,3,1,0},   //*64 - NOP $50
                          {"ADC", 5,3,0,2},   // 65 - ADC $50
                          {"ROR", 5,5,0,5},   // 66 - ROR $50
                          {"RRA", 5,5,1,9},   //*67 - RRA $50
                          {"PLA", AM_IMPLIED,4,0,2},   // 68 - PLA
                          {"ADC", AM_IMMEDIATE,2,0,2},   // 69 - ADC #$20
                          {"ROR", AM_IMPLIED,2,0,2},   // 6A - ROR
                          {"ARR", 2,5,1,9},   // 6B - ARR $00
                          {"JMP", 4,5,0,9},   // 6C - JMP ($4000)
                          {"ADC", 3,4,0,2},   // 6D - ADC $3000
                          {"ROR", 3,6,0,5},   // 6E - ROR $3000
                          {"RRA", 3,6,1,9},   //*6F - RRA $3000

                          {"BVS",12,4,0,9},   // 70 - BVS $C000 / add +1 if branch on same page, +2 othervise
                          {"ADC",10,6,0,2},   // 71 - ADC ($A0),X / add +1 if page boundary is crossed
                          {"ABS", AM_IMPLIED,2,1,1},   //*72 - ABS
                          {"RRA",10,9,1,9},   //*73 - RRA ($A0),Y
                          {"NOP", 6,4,1,0},   //*74 - NOP $60,X
                          {"ADC", 6,4,0,2},   // 75 - ADC $60,X
                          {"ROR", 6,6,0,5},   // 76 - ROR $60,X
                          {"RRA", 6,6,1,9},   //*77 - RRA $60,X
                          {"SEI", AM_IMPLIED,2,0,9},   // 78 - SEI
                          {"ADC", 9,5,0,2},   // 79 - ADC $9000,Y / add +1 if page boundary is crossed
                          {"NOP", AM_IMPLIED,2,1,0},   //*7A - NOP
                          {"RRA", 9,8,1,9},   //*7B - RRA $9000,Y
                          {"NOP", 8,5,1,0},   //*7C - NOP $8000,X
                          {"ADC", 8,5,0,2},   // 7D - ADC $8000,X / add +1 if page boundary is crossed
                          {"ROR", 8,7,0,5},   // 7E - ROR $8000,X
                          {"RRA", 8,8,1,9},   //*7F - RRA $8000,X

                          {"NOP", AM_IMMEDIATE,2,1,0},   //*80 - NOP #$00
                          {"STA",11,6,0,5},   // 81 - STA ($B0,X)
                          {"NOP", AM_IMMEDIATE,2,1,0},   //*82 - NOP ($B0,X)
                          {"AXX",11,8,1,9},   //*83 - AXX ($B0,X)
                          {"STY", 5,3,0,5},   // 84 - STY $50
                          {"STA", 5,3,0,5},   // 85 - STA $50
                          {"STX", 5,3,0,5},   // 86 - STX $50
                          {"SAX", 5,5,1,9},   //*87 - AXR $50
                          {"DEY", AM_IMPLIED,2,0,4},   // 88 - DEY
                          {"NOP", AM_IMMEDIATE,2,1,0},   //*89 - NOP #$20
                          {"TXA", AM_IMPLIED,2,0,2},   // 8A - TXA
                          {"XAA", AM_IMMEDIATE,2,1,9},   //*8B - TAN #$20
                          {"STY", 3,4,0,5},   // 8C - STY $3000
                          {"STA", 3,4,0,5},   // 8D - STA $3000
                          {"STX", 3,4,0,5},   // 8E - STX $3000
                          {"SAX", 3,4,1,9},   //*8F - SAX $3000

                          {"BCC",12,4,0,9},   // 90 - BNE $C000 / add +1 if branch on same page, +2 othervise
                          {"STA",10,6,0,5},   // 91 - STA ($A0),Y
                          {"ABS", AM_IMPLIED,2,1,9},   //*92 - ABS
                          {"AHX",10,7,1,9},   //*93 - AXI ($A0),Y - hmm????????????????
                          {"STY", 6,4,0,5},   // 94 - STY $60,X
                          {"STA", 6,4,0,5},   // 95 - STA $60,X
                          {"STX", 7,4,0,5},   // 96 - STX $70,Y
                          {"AXY", 7,4,1,9},   //*97 - AXY $70,X
                          {"TYA", AM_IMPLIED,2,0,2},   // 98 - TYA
                          {"STA", 9,5,0,5},   // 99 - STA $9000,Y
                          {"TXS", AM_IMPLIED,2,0,9},   // 9A - TXS
                          {"TAS", 9,6,1,9},   //*9B - AXS $9000,Y
                          {"SHY", 9,4,1,9},   //*9C - AYI/SHY $0000,X
                          {"STA", 8,5,0,5},   // 9D - STA $8000,X
                          {"SHX", 9,4,1,9},   //*9E - SXI/SHX $0000,Y
                          {"AHX", 9,4,1,9},   //*9F - AXI/SHA $0000,Y

                          {"LDY", AM_IMMEDIATE,2,0,4},   // A0 - LDY #$20
                          {"LDA",11,6,0,2},   // A1 - LDA ($B0,X)
                          {"LDX", AM_IMMEDIATE,2,0,3},   // A2 - LDX #$20
                          {"LAX",11,6,1,6},   //*A3 - LAX ($B0,X)
                          {"LDY", 5,3,0,4},   // A4 - LDY $50
                          {"LDA", 5,3,0,2},   // A5 - LDA $50
                          {"LDX", 5,3,0,3},   // A6 - LDX $50
                          {"LAX", 5,3,1,6},   //*A7 - LAX $50
                          {"TAY", AM_IMPLIED,2,0,4},   // A8 - TAY
                          {"LDA", AM_IMMEDIATE,2,0,2},   // A9 - LDA #$20
                          {"TAX", AM_IMPLIED,2,0,3},   // AA - TAX
                          {"ANX", AM_IMMEDIATE,2,1,9},   //*AB - ANX #$20
                          {"LDY", 3,4,0,4},   // AC - LDY $3000
                          {"LDA", 3,4,0,2},   // AD - LDA $3000
                          {"LDX", 3,4,0,3},   // AE - LDX $3000
                          {"LAX", 3,4,1,6},   //*AF - LAX $3000

                          {"BCS",12,4,0,9},   // B0 - BCS $C000 / add +1 if branch on same page, +2 othervise
                          {"LDA",10,6,0,2},   // B1 - LDA ($A0),Y / add +1 if page boundary is crossed
                          {"ABS", AM_IMPLIED,2,1,1},   //*B2 - ABS
                          {"LAX",10,6,1,6},   //*B3 - LAX ($A0),Y
                          {"LDY", 6,4,0,4},   // B4 - LDY $60,X
                          {"LDA", 6,4,0,2},   // B5 - LDA $60,X
                          {"LDX", 7,4,0,3},   // B6 - LDX $70,Y
                          {"LAX", 7,4,1,6},   //*B7 - LAX $70,X
                          {"CLV", AM_IMPLIED,2,0,9},   // B8 - CLV
                          {"LDA", 9,5,0,2},   // B9 - LDA $9000,X / add +1 if page boundary is crossed
                          {"TSX", AM_IMPLIED,2,0,9},   // BA - TSX
                          {"TSA", 8,5,1,9},   //*BB - TSA $8000,X or Y???
                          {"LDY", 8,5,0,4},   // BC - LDY $8000,X / add +1 if page boundary is crossed
                          {"LDA", 8,5,0,2},   // BD - LDA $8000,X / add +1 if page boundary is crossed
                          {"LDX", 9,5,0,3},   // BE - LDX $9000,Y / add +1 if page boundary is crossed
                          {"LAX", 9,5,1,6},   //*BF - LAX $9000,Y

                          {"CPY", AM_IMMEDIATE,2,0,9},   // C0 - CPY #$20
                          {"CMP",11,6,0,7},   // C1 - CMP ($B0,X)
                          {"NOP",11,2,1,0},   //*C2 - NOP ($B0,X)
                          {"DCP",11,8,1,9},   //*C3 - DCP ($B0,X)
                          {"CPY", 5,3,0,9},   // C4 - CPY $50
                          {"CMP", 5,3,0,7},   // C5 - CMP $50
                          {"DEC", 5,5,0,5},   // C6 - DEC $50
                          {"DCP", 5,5,1,9},   //*C7 - DCP $50
                          {"INY", AM_IMPLIED,2,0,4},   // C8 - INY
                          {"CMP", AM_IMMEDIATE,2,0,9},   // C9 - CMP #$20
                          {"DEX", AM_IMPLIED,2,0,3},   // CA - DEX
                          {"XAS", AM_IMMEDIATE,2,1,9},   //*CB - XAS #$20
                          {"CPY", 3,4,0,9},   // CC - CPY $3000
                          {"CMP", 3,4,0,7},   // CD - CMP $3000
                          {"DEC", 3,6,0,5},   // CE - DEC $3000
                          {"DCP", 3,6,1,9},   //*CF - DCP $3000

                          {"BNE",12,4,0,9},   // D0 - BNE $C000 / add +1 if branch on same page, +2 othervise
                          {"CMP",10,6,0,7},   // D1 - CMP ($A0),Y / add +1 if page boundary is crossed
                          {"ABS", AM_IMPLIED,2,1,1},   //*D2 - ABS
                          {"DCP",10,9,1,9},   //*D3 - DCP ($A0),Y
                          {"NOP", 6,4,1,0},   //*D4 - NOP $60,X
                          {"CMP", 6,4,0,7},   // D5 - CMP $60,X
                          {"DEC", 6,6,0,5},   // D6 - DEC $60,X
                          {"DCP", 7,6,1,9},   //*D7 - DCP $70,X or Y??
                          {"CLD", AM_IMPLIED,2,0,9},   // D8 - CLD
                          {"CMP", 9,5,0,7},   // D9 - CMP $9000,Y / add +1 if page boundary is crossed
                          {"NOP", AM_IMPLIED,2,1,0},   //*DA - NOP
                          {"DCP", 9,8,0,9},   //*DB - DCP $9000,Y
                          {"NOP", 8,5,1,0},   //*DC - NOP $8000,X
                          {"CMP", 8,5,0,7},   // DD - CMP $8000,X / add +1 if page boundary is crossed
                          {"DEC", 8,7,0,5},   // DE - DEC $8000,X
                          {"DCP", 8,8,1,9},   //*DF - DCP $8000,X

                          {"CPX", AM_IMMEDIATE,2,0,9},   // E0 - CPX #$20
                          {"SBC",11,6,0,2},   // E1 - SBC ($B0,X)
                          {"NOP",11,2,1,0},   //*E2 - NOP ($B0,X)
                          {"ISC",11,8,1,9},   //*E3 - ISC ($B0,X)
                          {"CPX", 5,3,0,9},   // E4 - CPX $50
                          {"SBC", 5,3,0,2},   // E5 - SBC $50
                          {"INC", 5,5,0,5},   // E6 - INC $50
                          {"ISC", 5,5,1,9},   //*E7 - ISC $50
                          {"INX", AM_IMPLIED,2,0,3},   // E8 - INX
                          {"SBC", AM_IMMEDIATE,2,0,2},   // E9 - SBC #$20
                          {"NOP", AM_IMPLIED,2,0,0},   // EA - NOP
                          {"SBC", AM_IMMEDIATE,2,1,2},   //*EB - SBC #$20
                          {"CPX", 3,4,0,9},   // EC - CPX $3000
                          {"SBC", 3,4,0,2},   // ED - SBC $3000
                          {"INC", 3,6,0,5},   // EE - INC $3000
                          {"ISC", 3,6,1,9},   //*EF - ISC $3000

                          {"BEQ",12,4,0,9},   // F0 - BEQ $C000 / add +1 if branch on same page, +2 othervise
                          {"SBC",10,6,0,2},   // F1 - SBC ($A0),Y
                          {"ABS", AM_IMPLIED,2,1,1},   //*F2 - ABS
                          {"ISC",10,9,1,9},   //*F3 - ISC ($A0),Y
                          {"NOP", 6,4,1,0},   //*F4 - NOP $60,X
                          {"SBC", 6,5,0,2},   // F5 - SBC $60,X / add +1 if page boundary is crossed
                          {"INC", 6,6,0,5},   // F6 - INC $60,X
                          {"ISC", 6,6,1,9},   //*F7 - ISC $60,X
                          {"SED", AM_IMPLIED,2,0,9},   // F8 - SED
                          {"SBC", 9,5,0,2},   // F9 - SBC $9000,Y / +1 on boundary crossing
                          {"NOP", AM_IMPLIED,2,1,0},   //*FA - NOP
                          {"ISC", 9,8,1,9},   //*FB - ISC $9000,Y
                          {"NOP", 8,5,1,0},   //*FC - NOP $8000,X
                          {"SBC", 8,5,0,2},   // FD - SBC $8000,X / add +1 if page boundary is crossed
                          {"INC", 8,7,0,5},   // FE - INC $8000,X
                          {"ISC", 8,8,1,9},   //*FF - ISC $8000,X
                        };
