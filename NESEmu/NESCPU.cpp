#include "stdafx.h"
#include "NESCPU.h"

using namespace std;

byte A, X, Y;
/* 0. carry, 1. zero, 2. decimal [unavail], 3. interrupt disable, 
   4. breakpoint, 5. overflow, 6. negative sign*/
bitset<8> S;
word PC, SP;

byte memory[0xFFFF];
byte* stack;

byte OP_CYCLES[0xFF];
void(NESCPU::*ADDR_MODE[0xFF])();
void(NESCPU::*OP_INSTR[0xFF])();

byte* OPERAND_ADDR;

NESCPU::NESCPU()
{
}

NESCPU::~NESCPU()
{
}

void NESCPU::init()
{
	A, X, Y, S = 0;
	PC = 0; // reset vector!
	SP = 0x00FF;
	stack = memory + 0x0100;

	/* DECLARE OPCODE PROPS */
	OP_CYCLES[0x00] = 7; OP_INSTR[0x00] = BRK; ADDR_MODE[0x00] = implied;
	OP_CYCLES[0x01] = 6; OP_INSTR[0x01] = ORA; ADDR_MODE[0x01] = indx;
	OP_CYCLES[0x02] = 2; OP_INSTR[0x02] = NOP; ADDR_MODE[0x02] = implied;
	OP_CYCLES[0x03] = 2; OP_INSTR[0x03] = NOP; ADDR_MODE[0x03] = implied;
	OP_CYCLES[0x04] = 3; OP_INSTR[0x04] = NOP; ADDR_MODE[0x04] = zp;
	OP_CYCLES[0x05] = 3; OP_INSTR[0x05] = ORA; ADDR_MODE[0x05] = zp;
	OP_CYCLES[0x06] = 5; OP_INSTR[0x06] = ASL; ADDR_MODE[0x06] = zp;
	OP_CYCLES[0x07] = 2; OP_INSTR[0x07] = NOP; ADDR_MODE[0x07] = implied;
	OP_CYCLES[0x08] = 3; OP_INSTR[0x08] = PHP; ADDR_MODE[0x08] = implied;
	OP_CYCLES[0x09] = 3; OP_INSTR[0x09] = ORA; ADDR_MODE[0x09] = immediate;
	OP_CYCLES[0x0a] = 2; OP_INSTR[0x0a] = ASL; ADDR_MODE[0x0a] = implied;
	OP_CYCLES[0x0b] = 2; OP_INSTR[0x0b] = NOP; ADDR_MODE[0x0b] = implied;
	OP_CYCLES[0x0c] = 4; OP_INSTR[0x0c] = NOP; ADDR_MODE[0x0c] = abs;
	OP_CYCLES[0x0d] = 4; OP_INSTR[0x0d] = ORA; ADDR_MODE[0x0d] = abs;
	OP_CYCLES[0x0e] = 6; OP_INSTR[0x0e] = ASL; ADDR_MODE[0x0e] = abs;
	OP_CYCLES[0x0f] = 2; OP_INSTR[0x0f] = NOP; ADDR_MODE[0x0f] = implied;
	OP_CYCLES[0x10] = 2; OP_INSTR[0x10] = BPL; ADDR_MODE[0x10] = relative;
	OP_CYCLES[0x11] = 5; OP_INSTR[0x11] = ORA; ADDR_MODE[0x11] = indy;
	OP_CYCLES[0x12] = 3; OP_INSTR[0x12] = NOP; ADDR_MODE[0x12] = indzp;
	OP_CYCLES[0x13] = 2; OP_INSTR[0x13] = NOP; ADDR_MODE[0x13] = implied;
	OP_CYCLES[0x14] = 3; OP_INSTR[0x14] = NOP; ADDR_MODE[0x14] = zp;
	OP_CYCLES[0x15] = 4; OP_INSTR[0x15] = ORA; ADDR_MODE[0x15] = zpx;
	OP_CYCLES[0x16] = 6; OP_INSTR[0x16] = ASL; ADDR_MODE[0x16] = zpx;
	OP_CYCLES[0x17] = 2; OP_INSTR[0x17] = NOP; ADDR_MODE[0x17] = implied;
	OP_CYCLES[0x18] = 2; OP_INSTR[0x18] = CLC; ADDR_MODE[0x18] = implied;
	OP_CYCLES[0x19] = 4; OP_INSTR[0x19] = ORA; ADDR_MODE[0x19] = absy;
	OP_CYCLES[0x1a] = 2; OP_INSTR[0x1a] = NOP; ADDR_MODE[0x1a] = implied;
	OP_CYCLES[0x1b] = 2; OP_INSTR[0x1b] = NOP; ADDR_MODE[0x1b] = implied;
	OP_CYCLES[0x1c] = 4; OP_INSTR[0x1c] = NOP; ADDR_MODE[0x1c] = abs;
	OP_CYCLES[0x1d] = 4; OP_INSTR[0x1d] = ORA; ADDR_MODE[0x1d] = absx;
	OP_CYCLES[0x1e] = 7; OP_INSTR[0x1e] = ASL; ADDR_MODE[0x1e] = absx;
	OP_CYCLES[0x1f] = 2; OP_INSTR[0x1f] = NOP; ADDR_MODE[0x1f] = implied;
	OP_CYCLES[0x20] = 6; OP_INSTR[0x20] = JSR; ADDR_MODE[0x20] = abs;
	OP_CYCLES[0x21] = 6; OP_INSTR[0x21] = AND; ADDR_MODE[0x21] = indx;
	OP_CYCLES[0x22] = 2; OP_INSTR[0x22] = NOP; ADDR_MODE[0x22] = implied;
	OP_CYCLES[0x23] = 2; OP_INSTR[0x23] = NOP; ADDR_MODE[0x23] = implied;
	OP_CYCLES[0x24] = 3; OP_INSTR[0x24] = BIT; ADDR_MODE[0x24] = zp;
	OP_CYCLES[0x25] = 3; OP_INSTR[0x25] = AND; ADDR_MODE[0x25] = zp;
	OP_CYCLES[0x26] = 5; OP_INSTR[0x26] = ROL; ADDR_MODE[0x26] = zp;
	OP_CYCLES[0x27] = 2; OP_INSTR[0x27] = NOP; ADDR_MODE[0x27] = implied;
	OP_CYCLES[0x28] = 4; OP_INSTR[0x28] = PLP; ADDR_MODE[0x28] = implied;
	OP_CYCLES[0x29] = 3; OP_INSTR[0x29] = AND; ADDR_MODE[0x29] = immediate;
	OP_CYCLES[0x2a] = 2; OP_INSTR[0x2a] = ROL; ADDR_MODE[0x2a] = implied;
	OP_CYCLES[0x2b] = 2; OP_INSTR[0x2b] = NOP; ADDR_MODE[0x2b] = implied;
	OP_CYCLES[0x2c] = 4; OP_INSTR[0x2c] = BIT; ADDR_MODE[0x2c] = abs;
	OP_CYCLES[0x2d] = 4; OP_INSTR[0x2d] = AND; ADDR_MODE[0x2d] = abs;
	OP_CYCLES[0x2e] = 6; OP_INSTR[0x2e] = ROL; ADDR_MODE[0x2e] = abs;
	OP_CYCLES[0x2f] = 2; OP_INSTR[0x2f] = NOP; ADDR_MODE[0x2f] = implied;
	OP_CYCLES[0x30] = 2; OP_INSTR[0x30] = BMI; ADDR_MODE[0x30] = relative;
	OP_CYCLES[0x31] = 5; OP_INSTR[0x31] = AND; ADDR_MODE[0x31] = indy;
	OP_CYCLES[0x32] = 3; OP_INSTR[0x32] = NOP; ADDR_MODE[0x32] = indzp;
	OP_CYCLES[0x33] = 2; OP_INSTR[0x33] = NOP; ADDR_MODE[0x33] = implied;
	OP_CYCLES[0x34] = 4; OP_INSTR[0x34] = NOP; ADDR_MODE[0x34] = zpx;
	OP_CYCLES[0x35] = 4; OP_INSTR[0x35] = AND; ADDR_MODE[0x35] = zpx;
	OP_CYCLES[0x36] = 6; OP_INSTR[0x36] = ROL; ADDR_MODE[0x36] = zpx;
	OP_CYCLES[0x37] = 2; OP_INSTR[0x37] = NOP; ADDR_MODE[0x37] = implied;
	OP_CYCLES[0x38] = 2; OP_INSTR[0x38] = SEC; ADDR_MODE[0x38] = implied;
	OP_CYCLES[0x39] = 4; OP_INSTR[0x39] = AND; ADDR_MODE[0x39] = absy;
	OP_CYCLES[0x3a] = 2; OP_INSTR[0x3a] = NOP; ADDR_MODE[0x3a] = implied;
	OP_CYCLES[0x3b] = 2; OP_INSTR[0x3b] = NOP; ADDR_MODE[0x3b] = implied;
	OP_CYCLES[0x3c] = 4; OP_INSTR[0x3c] = NOP; ADDR_MODE[0x3c] = absx;
	OP_CYCLES[0x3d] = 4; OP_INSTR[0x3d] = AND; ADDR_MODE[0x3d] = absx;
	OP_CYCLES[0x3e] = 7; OP_INSTR[0x3e] = ROL; ADDR_MODE[0x3e] = absx;
	OP_CYCLES[0x3f] = 2; OP_INSTR[0x3f] = NOP; ADDR_MODE[0x3f] = implied;
	OP_CYCLES[0x40] = 6; OP_INSTR[0x40] = RTI; ADDR_MODE[0x40] = implied;
	OP_CYCLES[0x41] = 6; OP_INSTR[0x41] = EOR; ADDR_MODE[0x41] = indx;
	OP_CYCLES[0x42] = 2; OP_INSTR[0x42] = NOP; ADDR_MODE[0x42] = implied;
	OP_CYCLES[0x43] = 2; OP_INSTR[0x43] = NOP; ADDR_MODE[0x43] = implied;
	OP_CYCLES[0x44] = 2; OP_INSTR[0x44] = NOP; ADDR_MODE[0x44] = implied;
	OP_CYCLES[0x45] = 3; OP_INSTR[0x45] = EOR; ADDR_MODE[0x45] = zp;
	OP_CYCLES[0x46] = 5; OP_INSTR[0x46] = LSR; ADDR_MODE[0x46] = zp;
	OP_CYCLES[0x47] = 2; OP_INSTR[0x47] = NOP; ADDR_MODE[0x47] = implied;
	OP_CYCLES[0x48] = 3; OP_INSTR[0x48] = PHA; ADDR_MODE[0x48] = implied;
	OP_CYCLES[0x49] = 3; OP_INSTR[0x49] = EOR; ADDR_MODE[0x49] = immediate;
	OP_CYCLES[0x4a] = 2; OP_INSTR[0x4a] = LSR; ADDR_MODE[0x4a] = implied;
	OP_CYCLES[0x4b] = 2; OP_INSTR[0x4b] = NOP; ADDR_MODE[0x4b] = implied;
	OP_CYCLES[0x4c] = 3; OP_INSTR[0x4c] = JMP; ADDR_MODE[0x4c] = abs;
	OP_CYCLES[0x4d] = 4; OP_INSTR[0x4d] = EOR; ADDR_MODE[0x4d] = abs;
	OP_CYCLES[0x4e] = 6; OP_INSTR[0x4e] = LSR; ADDR_MODE[0x4e] = abs;
	OP_CYCLES[0x4f] = 2; OP_INSTR[0x4f] = NOP; ADDR_MODE[0x4f] = implied;
	OP_CYCLES[0x50] = 2; OP_INSTR[0x50] = BVC; ADDR_MODE[0x50] = relative;
	OP_CYCLES[0x51] = 5; OP_INSTR[0x51] = EOR; ADDR_MODE[0x51] = indy;
	OP_CYCLES[0x52] = 3; OP_INSTR[0x52] = NOP; ADDR_MODE[0x52] = indzp;
	OP_CYCLES[0x53] = 2; OP_INSTR[0x53] = NOP; ADDR_MODE[0x53] = implied;
	OP_CYCLES[0x54] = 2; OP_INSTR[0x54] = NOP; ADDR_MODE[0x54] = implied;
	OP_CYCLES[0x55] = 4; OP_INSTR[0x55] = EOR; ADDR_MODE[0x55] = zpx;
	OP_CYCLES[0x56] = 6; OP_INSTR[0x56] = LSR; ADDR_MODE[0x56] = zpx;
	OP_CYCLES[0x57] = 2; OP_INSTR[0x57] = NOP; ADDR_MODE[0x57] = implied;
	OP_CYCLES[0x58] = 2; OP_INSTR[0x58] = CLI; ADDR_MODE[0x58] = implied;
	OP_CYCLES[0x59] = 4; OP_INSTR[0x59] = EOR; ADDR_MODE[0x59] = absy;
	OP_CYCLES[0x5a] = 3; OP_INSTR[0x5a] = NOP; ADDR_MODE[0x5a] = implied;
	OP_CYCLES[0x5b] = 2; OP_INSTR[0x5b] = NOP; ADDR_MODE[0x5b] = implied;
	OP_CYCLES[0x5c] = 2; OP_INSTR[0x5c] = NOP; ADDR_MODE[0x5c] = implied;
	OP_CYCLES[0x5d] = 4; OP_INSTR[0x5d] = EOR; ADDR_MODE[0x5d] = absx;
	OP_CYCLES[0x5e] = 7; OP_INSTR[0x5e] = LSR; ADDR_MODE[0x5e] = absx;
	OP_CYCLES[0x5f] = 2; OP_INSTR[0x5f] = NOP; ADDR_MODE[0x5f] = implied;
	OP_CYCLES[0x60] = 6; OP_INSTR[0x60] = RTS; ADDR_MODE[0x60] = implied;
	OP_CYCLES[0x61] = 6; OP_INSTR[0x61] = ADC; ADDR_MODE[0x61] = indx;
	OP_CYCLES[0x62] = 2; OP_INSTR[0x62] = NOP; ADDR_MODE[0x62] = implied;
	OP_CYCLES[0x63] = 2; OP_INSTR[0x63] = NOP; ADDR_MODE[0x63] = implied;
	OP_CYCLES[0x64] = 3; OP_INSTR[0x64] = NOP; ADDR_MODE[0x64] = zp;
	OP_CYCLES[0x65] = 3; OP_INSTR[0x65] = ADC; ADDR_MODE[0x65] = zp;
	OP_CYCLES[0x66] = 5; OP_INSTR[0x66] = ROR; ADDR_MODE[0x66] = zp;
	OP_CYCLES[0x67] = 2; OP_INSTR[0x67] = NOP; ADDR_MODE[0x67] = implied;
	OP_CYCLES[0x68] = 4; OP_INSTR[0x68] = PLA; ADDR_MODE[0x68] = implied;
	OP_CYCLES[0x69] = 3; OP_INSTR[0x69] = ADC; ADDR_MODE[0x69] = immediate;
	OP_CYCLES[0x6a] = 2; OP_INSTR[0x6a] = ROR; ADDR_MODE[0x6a] = implied;
	OP_CYCLES[0x6b] = 2; OP_INSTR[0x6b] = NOP; ADDR_MODE[0x6b] = implied;
	OP_CYCLES[0x6c] = 5; OP_INSTR[0x6c] = JMP; ADDR_MODE[0x6c] = indirect;
	OP_CYCLES[0x6d] = 4; OP_INSTR[0x6d] = ADC; ADDR_MODE[0x6d] = abs;
	OP_CYCLES[0x6e] = 6; OP_INSTR[0x6e] = ROR; ADDR_MODE[0x6e] = abs;
	OP_CYCLES[0x6f] = 2; OP_INSTR[0x6f] = NOP; ADDR_MODE[0x6f] = implied;
	OP_CYCLES[0x70] = 2; OP_INSTR[0x70] = BVS; ADDR_MODE[0x70] = relative;
	OP_CYCLES[0x71] = 5; OP_INSTR[0x71] = ADC; ADDR_MODE[0x71] = indy;
	OP_CYCLES[0x72] = 3; OP_INSTR[0x72] = NOP; ADDR_MODE[0x72] = indzp;
	OP_CYCLES[0x73] = 2; OP_INSTR[0x73] = NOP; ADDR_MODE[0x73] = implied;
	OP_CYCLES[0x74] = 4; OP_INSTR[0x74] = NOP; ADDR_MODE[0x74] = zpx;
	OP_CYCLES[0x75] = 4; OP_INSTR[0x75] = ADC; ADDR_MODE[0x75] = zpx;
	OP_CYCLES[0x76] = 6; OP_INSTR[0x76] = ROR; ADDR_MODE[0x76] = zpx;
	OP_CYCLES[0x77] = 2; OP_INSTR[0x77] = NOP; ADDR_MODE[0x77] = implied;
	OP_CYCLES[0x78] = 2; OP_INSTR[0x78] = SEI; ADDR_MODE[0x78] = implied;
	OP_CYCLES[0x79] = 4; OP_INSTR[0x79] = ADC; ADDR_MODE[0x79] = absy;
	OP_CYCLES[0x7a] = 4; OP_INSTR[0x7a] = NOP; ADDR_MODE[0x7a] = implied;
	OP_CYCLES[0x7b] = 2; OP_INSTR[0x7b] = NOP; ADDR_MODE[0x7b] = implied;
	OP_CYCLES[0x7c] = 6; OP_INSTR[0x7c] = NOP; ADDR_MODE[0x7c] = indabsx;
	OP_CYCLES[0x7d] = 4; OP_INSTR[0x7d] = ADC; ADDR_MODE[0x7d] = absx;
	OP_CYCLES[0x7e] = 7; OP_INSTR[0x7e] = ROR; ADDR_MODE[0x7e] = absx;
	OP_CYCLES[0x7f] = 2; OP_INSTR[0x7f] = NOP; ADDR_MODE[0x7f] = implied;
	OP_CYCLES[0x80] = 2; OP_INSTR[0x80] = NOP; ADDR_MODE[0x80] = relative;
	OP_CYCLES[0x81] = 6; OP_INSTR[0x81] = STA; ADDR_MODE[0x81] = indx;
	OP_CYCLES[0x82] = 2; OP_INSTR[0x82] = NOP; ADDR_MODE[0x82] = implied;
	OP_CYCLES[0x83] = 2; OP_INSTR[0x83] = NOP; ADDR_MODE[0x83] = implied;
	OP_CYCLES[0x84] = 2; OP_INSTR[0x84] = STY; ADDR_MODE[0x84] = zp;
	OP_CYCLES[0x85] = 2; OP_INSTR[0x85] = STA; ADDR_MODE[0x85] = zp;
	OP_CYCLES[0x86] = 2; OP_INSTR[0x86] = STX; ADDR_MODE[0x86] = zp;
	OP_CYCLES[0x87] = 2; OP_INSTR[0x87] = NOP; ADDR_MODE[0x87] = implied;
	OP_CYCLES[0x88] = 2; OP_INSTR[0x88] = DEY; ADDR_MODE[0x88] = implied;
	OP_CYCLES[0x89] = 2; OP_INSTR[0x89] = NOP; ADDR_MODE[0x89] = immediate;
	OP_CYCLES[0x8a] = 2; OP_INSTR[0x8a] = TXA; ADDR_MODE[0x8a] = implied;
	OP_CYCLES[0x8b] = 2; OP_INSTR[0x8b] = NOP; ADDR_MODE[0x8b] = implied;
	OP_CYCLES[0x8c] = 4; OP_INSTR[0x8c] = STY; ADDR_MODE[0x8c] = abs;
	OP_CYCLES[0x8d] = 4; OP_INSTR[0x8d] = STA; ADDR_MODE[0x8d] = abs;
	OP_CYCLES[0x8e] = 4; OP_INSTR[0x8e] = STX; ADDR_MODE[0x8e] = abs;
	OP_CYCLES[0x8f] = 2; OP_INSTR[0x8f] = NOP; ADDR_MODE[0x8f] = implied;
	OP_CYCLES[0x90] = 2; OP_INSTR[0x90] = BCC; ADDR_MODE[0x90] = relative;
	OP_CYCLES[0x91] = 6; OP_INSTR[0x91] = STA; ADDR_MODE[0x91] = indy;
	OP_CYCLES[0x92] = 3; OP_INSTR[0x92] = NOP; ADDR_MODE[0x92] = indzp;
	OP_CYCLES[0x93] = 2; OP_INSTR[0x93] = NOP; ADDR_MODE[0x93] = implied;
	OP_CYCLES[0x94] = 4; OP_INSTR[0x94] = STY; ADDR_MODE[0x94] = zpx;
	OP_CYCLES[0x95] = 4; OP_INSTR[0x95] = STA; ADDR_MODE[0x95] = zpx;
	OP_CYCLES[0x96] = 4; OP_INSTR[0x96] = STX; ADDR_MODE[0x96] = zpy;
	OP_CYCLES[0x97] = 2; OP_INSTR[0x97] = NOP; ADDR_MODE[0x97] = implied;
	OP_CYCLES[0x98] = 2; OP_INSTR[0x98] = TYA; ADDR_MODE[0x98] = implied;
	OP_CYCLES[0x99] = 5; OP_INSTR[0x99] = STA; ADDR_MODE[0x99] = absy;
	OP_CYCLES[0x9a] = 2; OP_INSTR[0x9a] = TXS; ADDR_MODE[0x9a] = implied;
	OP_CYCLES[0x9b] = 2; OP_INSTR[0x9b] = NOP; ADDR_MODE[0x9b] = implied;
	OP_CYCLES[0x9c] = 4; OP_INSTR[0x9c] = NOP; ADDR_MODE[0x9c] = abs;
	OP_CYCLES[0x9d] = 5; OP_INSTR[0x9d] = STA; ADDR_MODE[0x9d] = absx;
	OP_CYCLES[0x9e] = 5; OP_INSTR[0x9e] = NOP; ADDR_MODE[0x9e] = absx;
	OP_CYCLES[0x9f] = 2; OP_INSTR[0x9f] = NOP; ADDR_MODE[0x9f] = implied;
	OP_CYCLES[0xa0] = 3; OP_INSTR[0xa0] = LDY; ADDR_MODE[0xa0] = immediate;
	OP_CYCLES[0xa1] = 6; OP_INSTR[0xa1] = LDA; ADDR_MODE[0xa1] = indx;
	OP_CYCLES[0xa2] = 3; OP_INSTR[0xa2] = LDX; ADDR_MODE[0xa2] = immediate;
	OP_CYCLES[0xa3] = 2; OP_INSTR[0xa3] = NOP; ADDR_MODE[0xa3] = implied;
	OP_CYCLES[0xa4] = 3; OP_INSTR[0xa4] = LDY; ADDR_MODE[0xa4] = zp;
	OP_CYCLES[0xa5] = 3; OP_INSTR[0xa5] = LDA; ADDR_MODE[0xa5] = zp;
	OP_CYCLES[0xa6] = 3; OP_INSTR[0xa6] = LDX; ADDR_MODE[0xa6] = zp;
	OP_CYCLES[0xa7] = 2; OP_INSTR[0xa7] = NOP; ADDR_MODE[0xa7] = implied;
	OP_CYCLES[0xa8] = 2; OP_INSTR[0xa8] = TAY; ADDR_MODE[0xa8] = implied;
	OP_CYCLES[0xa9] = 3; OP_INSTR[0xa9] = LDA; ADDR_MODE[0xa9] = immediate;
	OP_CYCLES[0xaa] = 2; OP_INSTR[0xaa] = TAX; ADDR_MODE[0xaa] = implied;
	OP_CYCLES[0xab] = 2; OP_INSTR[0xab] = NOP; ADDR_MODE[0xab] = implied;
	OP_CYCLES[0xac] = 4; OP_INSTR[0xac] = LDY; ADDR_MODE[0xac] = abs;
	OP_CYCLES[0xad] = 4; OP_INSTR[0xad] = LDA; ADDR_MODE[0xad] = abs;
	OP_CYCLES[0xae] = 4; OP_INSTR[0xae] = LDX; ADDR_MODE[0xae] = abs;
	OP_CYCLES[0xaf] = 2; OP_INSTR[0xaf] = NOP; ADDR_MODE[0xaf] = implied;
	OP_CYCLES[0xb0] = 2; OP_INSTR[0xb0] = BCS; ADDR_MODE[0xb0] = relative;
	OP_CYCLES[0xb1] = 5; OP_INSTR[0xb1] = LDA; ADDR_MODE[0xb1] = indy;
	OP_CYCLES[0xb2] = 3; OP_INSTR[0xb2] = NOP; ADDR_MODE[0xb2] = indzp;
	OP_CYCLES[0xb3] = 2; OP_INSTR[0xb3] = NOP; ADDR_MODE[0xb3] = implied;
	OP_CYCLES[0xb4] = 4; OP_INSTR[0xb4] = LDY; ADDR_MODE[0xb4] = zpx;
	OP_CYCLES[0xb5] = 4; OP_INSTR[0xb5] = LDA; ADDR_MODE[0xb5] = zpx;
	OP_CYCLES[0xb6] = 4; OP_INSTR[0xb6] = LDX; ADDR_MODE[0xb6] = zpy;
	OP_CYCLES[0xb7] = 2; OP_INSTR[0xb7] = NOP; ADDR_MODE[0xb7] = implied;
	OP_CYCLES[0xb8] = 2; OP_INSTR[0xb8] = CLV; ADDR_MODE[0xb8] = implied;
	OP_CYCLES[0xb9] = 4; OP_INSTR[0xb9] = LDA; ADDR_MODE[0xb9] = absy;
	OP_CYCLES[0xba] = 2; OP_INSTR[0xba] = TSX; ADDR_MODE[0xba] = implied;
	OP_CYCLES[0xbb] = 2; OP_INSTR[0xbb] = NOP; ADDR_MODE[0xbb] = implied;
	OP_CYCLES[0xbc] = 4; OP_INSTR[0xbc] = LDY; ADDR_MODE[0xbc] = absx;
	OP_CYCLES[0xbd] = 4; OP_INSTR[0xbd] = LDA; ADDR_MODE[0xbd] = absx;
	OP_CYCLES[0xbe] = 4; OP_INSTR[0xbe] = LDX; ADDR_MODE[0xbe] = absy;
	OP_CYCLES[0xbf] = 2; OP_INSTR[0xbf] = NOP; ADDR_MODE[0xbf] = implied;
	OP_CYCLES[0xc0] = 3; OP_INSTR[0xc0] = CPY; ADDR_MODE[0xc0] = immediate;
	OP_CYCLES[0xc1] = 6; OP_INSTR[0xc1] = CMP; ADDR_MODE[0xc1] = indx;
	OP_CYCLES[0xc2] = 2; OP_INSTR[0xc2] = NOP; ADDR_MODE[0xc2] = implied;
	OP_CYCLES[0xc3] = 2; OP_INSTR[0xc3] = NOP; ADDR_MODE[0xc3] = implied;
	OP_CYCLES[0xc4] = 3; OP_INSTR[0xc4] = CPY; ADDR_MODE[0xc4] = zp;
	OP_CYCLES[0xc5] = 3; OP_INSTR[0xc5] = CMP; ADDR_MODE[0xc5] = zp;
	OP_CYCLES[0xc6] = 5; OP_INSTR[0xc6] = DEC; ADDR_MODE[0xc6] = zp;
	OP_CYCLES[0xc7] = 2; OP_INSTR[0xc7] = NOP; ADDR_MODE[0xc7] = implied;
	OP_CYCLES[0xc8] = 2; OP_INSTR[0xc8] = INY; ADDR_MODE[0xc8] = implied;
	OP_CYCLES[0xc9] = 3; OP_INSTR[0xc9] = CMP; ADDR_MODE[0xc9] = immediate;
	OP_CYCLES[0xca] = 2; OP_INSTR[0xca] = DEX; ADDR_MODE[0xca] = implied;
	OP_CYCLES[0xcb] = 2; OP_INSTR[0xcb] = NOP; ADDR_MODE[0xcb] = implied;
	OP_CYCLES[0xcc] = 4; OP_INSTR[0xcc] = CPY; ADDR_MODE[0xcc] = abs;
	OP_CYCLES[0xcd] = 4; OP_INSTR[0xcd] = CMP; ADDR_MODE[0xcd] = abs;
	OP_CYCLES[0xce] = 6; OP_INSTR[0xce] = DEC; ADDR_MODE[0xce] = abs;
	OP_CYCLES[0xcf] = 2; OP_INSTR[0xcf] = NOP; ADDR_MODE[0xcf] = implied;
	OP_CYCLES[0xd0] = 2; OP_INSTR[0xd0] = BNE; ADDR_MODE[0xd0] = relative;
	OP_CYCLES[0xd1] = 5; OP_INSTR[0xd1] = CMP; ADDR_MODE[0xd1] = indy;
	OP_CYCLES[0xd2] = 3; OP_INSTR[0xd2] = NOP; ADDR_MODE[0xd2] = indzp;
	OP_CYCLES[0xd3] = 2; OP_INSTR[0xd3] = NOP; ADDR_MODE[0xd3] = implied;
	OP_CYCLES[0xd4] = 2; OP_INSTR[0xd4] = NOP; ADDR_MODE[0xd4] = implied;
	OP_CYCLES[0xd5] = 4; OP_INSTR[0xd5] = CMP; ADDR_MODE[0xd5] = zpx;
	OP_CYCLES[0xd6] = 6; OP_INSTR[0xd6] = DEC; ADDR_MODE[0xd6] = zpx;
	OP_CYCLES[0xd7] = 2; OP_INSTR[0xd7] = NOP; ADDR_MODE[0xd7] = implied;
	OP_CYCLES[0xd8] = 2; OP_INSTR[0xd8] = CLD; ADDR_MODE[0xd8] = implied;
	OP_CYCLES[0xd9] = 4; OP_INSTR[0xd9] = CMP; ADDR_MODE[0xd9] = absy;
	OP_CYCLES[0xda] = 3; OP_INSTR[0xda] = NOP; ADDR_MODE[0xda] = implied;
	OP_CYCLES[0xdb] = 2; OP_INSTR[0xdb] = NOP; ADDR_MODE[0xdb] = implied;
	OP_CYCLES[0xdc] = 2; OP_INSTR[0xdc] = NOP; ADDR_MODE[0xdc] = implied;
	OP_CYCLES[0xdd] = 4; OP_INSTR[0xdd] = CMP; ADDR_MODE[0xdd] = absx;
	OP_CYCLES[0xde] = 7; OP_INSTR[0xde] = DEC; ADDR_MODE[0xde] = absx;
	OP_CYCLES[0xdf] = 2; OP_INSTR[0xdf] = NOP; ADDR_MODE[0xdf] = implied;
	OP_CYCLES[0xe0] = 3; OP_INSTR[0xe0] = CPX; ADDR_MODE[0xe0] = immediate;
	OP_CYCLES[0xe1] = 6; OP_INSTR[0xe1] = SBC; ADDR_MODE[0xe1] = indx;
	OP_CYCLES[0xe2] = 2; OP_INSTR[0xe2] = NOP; ADDR_MODE[0xe2] = implied;
	OP_CYCLES[0xe3] = 2; OP_INSTR[0xe3] = NOP; ADDR_MODE[0xe3] = implied;
	OP_CYCLES[0xe4] = 3; OP_INSTR[0xe4] = CPX; ADDR_MODE[0xe4] = zp;
	OP_CYCLES[0xe5] = 3; OP_INSTR[0xe5] = SBC; ADDR_MODE[0xe5] = zp;
	OP_CYCLES[0xe6] = 5; OP_INSTR[0xe6] = INC; ADDR_MODE[0xe6] = zp;
	OP_CYCLES[0xe7] = 2; OP_INSTR[0xe7] = NOP; ADDR_MODE[0xe7] = implied;
	OP_CYCLES[0xe8] = 2; OP_INSTR[0xe8] = INX; ADDR_MODE[0xe8] = implied;
	OP_CYCLES[0xe9] = 3; OP_INSTR[0xe9] = SBC; ADDR_MODE[0xe9] = immediate;
	OP_CYCLES[0xea] = 2; OP_INSTR[0xea] = NOP; ADDR_MODE[0xea] = implied;
	OP_CYCLES[0xeb] = 2; OP_INSTR[0xeb] = NOP; ADDR_MODE[0xeb] = implied;
	OP_CYCLES[0xec] = 4; OP_INSTR[0xec] = CPX; ADDR_MODE[0xec] = abs;
	OP_CYCLES[0xed] = 4; OP_INSTR[0xed] = SBC; ADDR_MODE[0xed] = abs;
	OP_CYCLES[0xee] = 6; OP_INSTR[0xee] = INC; ADDR_MODE[0xee] = abs;
	OP_CYCLES[0xef] = 2; OP_INSTR[0xef] = NOP; ADDR_MODE[0xef] = implied;
	OP_CYCLES[0xf0] = 2; OP_INSTR[0xf0] = BEQ; ADDR_MODE[0xf0] = relative;
	OP_CYCLES[0xf1] = 5; OP_INSTR[0xf1] = SBC; ADDR_MODE[0xf1] = indy;
	OP_CYCLES[0xf2] = 3; OP_INSTR[0xf2] = NOP; ADDR_MODE[0xf2] = indzp;
	OP_CYCLES[0xf3] = 2; OP_INSTR[0xf3] = NOP; ADDR_MODE[0xf3] = implied;
	OP_CYCLES[0xf4] = 2; OP_INSTR[0xf4] = NOP; ADDR_MODE[0xf4] = implied;
	OP_CYCLES[0xf5] = 4; OP_INSTR[0xf5] = SBC; ADDR_MODE[0xf5] = zpx;
	OP_CYCLES[0xf6] = 6; OP_INSTR[0xf6] = INC; ADDR_MODE[0xf6] = zpx;
	OP_CYCLES[0xf7] = 2; OP_INSTR[0xf7] = NOP; ADDR_MODE[0xf7] = implied;
	OP_CYCLES[0xf8] = 2; OP_INSTR[0xf8] = SED; ADDR_MODE[0xf8] = implied;
	OP_CYCLES[0xf9] = 4; OP_INSTR[0xf9] = SBC; ADDR_MODE[0xf9] = absy;
	OP_CYCLES[0xfa] = 4; OP_INSTR[0xfa] = NOP; ADDR_MODE[0xfa] = implied;
	OP_CYCLES[0xfb] = 2; OP_INSTR[0xfb] = NOP; ADDR_MODE[0xfb] = implied;
	OP_CYCLES[0xfc] = 2; OP_INSTR[0xfc] = NOP; ADDR_MODE[0xfc] = implied;
	OP_CYCLES[0xfd] = 4; OP_INSTR[0xfd] = SBC; ADDR_MODE[0xfd] = absx;
	OP_CYCLES[0xfe] = 7; OP_INSTR[0xfe] = INC; ADDR_MODE[0xfe] = absx;
	OP_CYCLES[0xff] = 2; OP_INSTR[0xff] = NOP; ADDR_MODE[0xff] = implied;
}

void NESCPU::exec()
{

}

void NESCPU::implied()
{

}
void NESCPU::immediate()
{

}
void NESCPU::abs()
{

}
void NESCPU::relative()
{

}
void NESCPU::indirect()
{

}
void NESCPU::absx()
{

}
void NESCPU::absy()
{

}
void NESCPU::zp()
{

}
void NESCPU::zpx()
{

}
void NESCPU::zpy()
{

}
void NESCPU::indx()
{

}
void NESCPU::indy()
{

}
void NESCPU::indabsx()
{

}
void NESCPU::indzp()
{

}

void NESCPU::ADC()
{
	A += *OPERAND_ADDR + S[FLAG_CARRY];
	S[FLAG_CARRY] = A > 0xFF;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_OVERFLOW] = A > 0x7F;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Add Memory to Accumulator with Carry
void NESCPU::AND()
{
	A &= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// "AND" Memory with Accumulator
void NESCPU::ASL()
{
	S[FLAG_CARRY] = bitset<8>(A)[7];
	*OPERAND_ADDR <<= 1;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Shift Left One Bit (Memory or Accumulator)

void NESCPU::BCC()
{
	if (!S[FLAG_CARRY])
		PC += *OPERAND_ADDR;
}// Branch on Carry Clear
void NESCPU::BCS()
{
	if (S[FLAG_CARRY])
		PC += *OPERAND_ADDR;
}// Branch on Carry Set
void NESCPU::BEQ()
{
	if (S[FLAG_ZERO])
		PC += *OPERAND_ADDR;
}// Branch on Result Zero
void NESCPU::BIT()
{
	// TODO
}// Test Bits in Memory with Accumulator
void NESCPU::BMI()
{
	if (S[FLAG_NEGATIVE_SIGN])
		PC += *OPERAND_ADDR;
}// Branch on Result Minus
void NESCPU::BNE()
{
	if (!S[FLAG_ZERO])
		PC += *OPERAND_ADDR;
}// Branch on Result not Zero
void NESCPU::BPL()
{
	if (!S[FLAG_NEGATIVE_SIGN])
		PC += *OPERAND_ADDR;
}// Branch on Result Plus
void NESCPU::BRK()
{
	S[FLAG_BREAKPOINT] = 1;
	PC++;
	stack[SP--] = PC >> 4;
	stack[SP--] = PC & 0xFF;
	stack[SP--] = (byte) S.to_ulong();
	PC = memory[0xFFFE] | (memory[0xFFFF] << 8);
}// Force Break
void NESCPU::BVC()
{
	if (!S[FLAG_OVERFLOW])
		PC += *OPERAND_ADDR;
}// Branch on Overflow Clear
void NESCPU::BVS()
{
	if (S[FLAG_OVERFLOW])
		PC += *OPERAND_ADDR;
}// Branch on Overflow Set

void NESCPU::CLC()
{
	S[FLAG_CARRY] = 0;
}// Clear Carry Flag
void NESCPU::CLD()
{
	S[FLAG_DECIMAL] = 0;
}// Clear Decimal Mode
void NESCPU::CLI()
{
	S[FLAG_INTERRUPT_DISABLE] = 0;
}// Clear interrupt Disable Bit
void NESCPU::CLV()
{
	S[FLAG_OVERFLOW] = 0;
}// Clear Overflow Flag
void NESCPU::CMP()
{
	S[FLAG_CARRY] = A >= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == *OPERAND_ADDR;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A - *OPERAND_ADDR)[7];
}// Compare Memory and Accumulator
void NESCPU::CPX()
{
	S[FLAG_CARRY] = X >= *OPERAND_ADDR;
	S[FLAG_ZERO] = X == *OPERAND_ADDR;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X - *OPERAND_ADDR)[7];
}// Compare Memory and Index X
void NESCPU::CPY()
{
	S[FLAG_CARRY] = Y >= *OPERAND_ADDR;
	S[FLAG_ZERO] = Y == *OPERAND_ADDR;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y - *OPERAND_ADDR)[7];
}// Compare Memory and Index Y

void NESCPU::DEC()
{
	--*OPERAND_ADDR;
	S[FLAG_ZERO] = *OPERAND_ADDR == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(*OPERAND_ADDR)[7];
}// Decrement Memory by One
void NESCPU::DEX()
{
	--X;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Decrement Index X by One
void NESCPU::DEY()
{
	--Y;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Decrement Index Y by One
void NESCPU::EOR()
{
	A ^= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// "Exclusive-Or" Memory with Accumulator

void NESCPU::INC()
{
	++*OPERAND_ADDR;
	S[FLAG_ZERO] = *OPERAND_ADDR == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(*OPERAND_ADDR)[7];
}// Increment Memory by One
void NESCPU::INX()
{
	++X;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Increment Index X by One
void NESCPU::INY()
{
	++Y;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Increment Index Y by One

void NESCPU::JMP()
{
	PC = *OPERAND_ADDR;
}// Jump to New Location
void NESCPU::JSR()
{
	stack[SP--] = PC >> 4;
	stack[SP--] = PC & 0xFF;
	PC = *OPERAND_ADDR;
}// Jump to New Location Saving Return Address

void NESCPU::LDA()
{
	A = *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Load Accumulator with Memory
void NESCPU::LDX()
{
	X = *OPERAND_ADDR;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Load Index X with Memory
void NESCPU::LDY()
{
	Y = *OPERAND_ADDR;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Load Index Y with Memory 
void NESCPU::LSR()
{
	S[FLAG_CARRY] = bitset<8>(*OPERAND_ADDR)[0];
	*OPERAND_ADDR >>= 1;
	S[FLAG_ZERO] = *OPERAND_ADDR == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(*OPERAND_ADDR)[7];
}// Shift Right One Bit(Memory or Accumulator)

void NESCPU::NOP()
{
	// this is a waste of code
}// No Operation

void NESCPU::ORA()
{
	A |= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// "OR" Memory with Accumulator

void NESCPU::PHA()
{
	stack[SP--] = A;
}// Push Accumulator on Stack
void NESCPU::PHP()
{
	stack[SP--] = (byte)S.to_ulong();
}// Push Processor Status on Stack
void NESCPU::PLA()
{
	A = stack[++SP];
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Pull Accumulator from Stack
void NESCPU::PLP()
{
	S = stack[++SP];
}// Pull Processor Status from Stack

void NESCPU::ROL()
{
	bitset<8> tmp(*OPERAND_ADDR);
	bool old = tmp[7];
	tmp <<= 1;
	tmp[0] = S[FLAG_CARRY];
	S[FLAG_CARRY] = old;
	*OPERAND_ADDR = (byte)tmp.to_ulong();
}// Rotate One Bit Left(Memory or Accumulator)
void NESCPU::ROR()
{
	bitset<8> tmp(*OPERAND_ADDR);
	bool old = tmp[0];
	tmp >>= 1;
	tmp[7] = S[FLAG_CARRY];
	S[FLAG_CARRY] = old;
	*OPERAND_ADDR = (byte)tmp.to_ulong();
}// Rotate One Bit Right(Memory or Accumulator)
void NESCPU::RTI()
{
	PLP();
	RTS();
}// Return from Interrupt
void NESCPU::RTS()
{
	PC = stack[++SP] | (stack[++SP] << 8);
}// Return from Subroutine

void NESCPU::SBC()
{
	A -= *OPERAND_ADDR - !S[FLAG_CARRY];
	S[FLAG_CARRY] = !bitset<8>(A)[7];
	S[FLAG_ZERO] = A == 0;
	S[FLAG_OVERFLOW] = A > 0x7F;
	S[FLAG_NEGATIVE_SIGN] = !S[FLAG_CARRY];
}// Subtract Memory from Accumulator with Borrow
void NESCPU::SEC()
{
	S[FLAG_CARRY] = 1;
}// Set Carry Flag
void NESCPU::SED()
{
	S[FLAG_DECIMAL] = 1;
}// Set Decimal Mode
void NESCPU::SEI()
{
	S[FLAG_INTERRUPT_DISABLE] = 1;
}// Set Interrupt Disable Status
void NESCPU::STA()
{
	*OPERAND_ADDR = A;
}// Store Accumulator in Memory
void NESCPU::STX()
{
	*OPERAND_ADDR = X;
}// Store Index X in Memory
void NESCPU::STY()
{
	*OPERAND_ADDR = Y;
}// Store Index Y in Memory

void NESCPU::TAX()
{
	X = A;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Transfer Accumulator to Index X
void NESCPU::TAY()
{
	Y = A;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Transfer Accumulator to Index Y
void NESCPU::TSX()
{
	X = SP;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Transfer Stack Pointer to Index X
void NESCPU::TXA()
{
	A = X;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Transfer Index X to Accumulator
void NESCPU::TXS()
{
	SP = X;
}// Transfer Index X to Stack Pointer
void NESCPU::TYA()
{
	A = Y;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Transfer Index Y to Accumulator