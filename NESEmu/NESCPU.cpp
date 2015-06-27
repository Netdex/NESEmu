#include "stdafx.h"
#include "NESCPU.h"

using namespace std;

/* ADDRESSING MODE DECLARATIONS */
void NES_ADDR_MODE_implied();		// NO OPERAND ADDRESS
void NES_ADDR_MODE_immediate();	// [OP VALUE] given by inst (LDA #$0A)
void NES_ADDR_MODE_absolute();			// [OP ADDR] given by inst (LDA $31F6)
void NES_ADDR_MODE_relative();		// [REL ADDR VALUE] given by inst, then added as signed byte onto PC (BEQ $A7)
void NES_ADDR_MODE_indirect();		// [OP ADDR] given by inst, next byte added to make JMP address (JMP ($215F))
void NES_ADDR_MODE_absx();			// [REL OP ADDR] given by inst, then added to X reg to get op address (STA $3000,X)
void NES_ADDR_MODE_absy();
void NES_ADDR_MODE_zp();			// [OP ADDR] given by inst, 1 byte
void NES_ADDR_MODE_zpx();			// [OP ADDR] given by inst, add to X reg, wrap around 0xff!
void NES_ADDR_MODE_zpy();
void NES_ADDR_MODE_indx();			// [OP ADDR] taken from inst, addr is added to X reg with wrap
void NES_ADDR_MODE_indy();
void NES_ADDR_MODE_accum();		// [OP ADDR] is the accumulator

/* OPCODE DECLARATIONS */
void NES_OP_ADC();		// Add Memory to Accumulator with Carry
void NES_OP_AND();		// "AND" Memory with Accumulator
void NES_OP_ASL();	// Shift Left One Bit (Memory or Accumulator)
void NES_OP_BCC();		// Branch on Carry Clear
void NES_OP_BCS();		// Branch on Carry Set
void NES_OP_BEQ();		// Branch on Result Zero
void NES_OP_BIT();		// Test Bits in Memory with Accumulator
void NES_OP_BMI();		// Branch on Result Minus
void NES_OP_BNE();		// Branch on Result not Zero
void NES_OP_BPL();		// Branch on Result Plus
void NES_OP_BRK();				// Force Break
void NES_OP_BVC();		// Branch on Overflow Clear
void NES_OP_BVS();		// Branch on Overflow Set
void NES_OP_CLC();				// Clear Carry Flag
void NES_OP_CLD();				// Clear Decimal Mode
void NES_OP_CLI();				// Clear interrupt Disable Bit
void NES_OP_CLV();				// Clear Overflow Flag
void NES_OP_CMP();		// Compare Memory and Accumulator
void NES_OP_CPX();		// Compare Memory and Index X
void NES_OP_CPY();		// Compare Memory and Index Y
void NES_OP_DEC();	// Decrement Memory by One
void NES_OP_DEX();				// Decrement Index X by One
void NES_OP_DEY();				// Decrement Index Y by One
void NES_OP_EOR();		// "Exclusive-Or" Memory with Accumulator
void NES_OP_INC();	// Increment Memory by One
void NES_OP_INX();				// Increment Index X by One
void NES_OP_INY();				// Increment Index Y by One
void NES_OP_JMP();		// Jump to New Location
void NES_OP_JSR();		// Jump to New Location Saving Return Address

void NES_OP_LDA();		// Load Accumulator with Memory
void NES_OP_LDX();		// Load Index X with Memory
void NES_OP_LDY();		// Load Index Y with Memory 
void NES_OP_LSR();	// Shift Right One Bit(Memory or Accumulator)
void NES_OP_NOP();				// No Operation
void NES_OP_ORA();		// "OR" Memory with Accumulator
void NES_OP_PHA();				// Push Accumulator on Stack
void NES_OP_PHP();				// Push Processor Status on Stack
void NES_OP_PLA();				// Pull Accumulator from Stack
void NES_OP_PLP();				// Pull Processor Status from Stack
void NES_OP_ROL();	// Rotate One Bit Left(Memory or Accumulator)
void NES_OP_ROR();	// Rotate One Bit Right(Memory or Accumulator)
void NES_OP_RTI();				// Return from Interrupt
void NES_OP_RTS();				// Return from Subroutine
void NES_OP_SBC();		// Subtract Memory from Accumulator with Borrow
void NES_OP_SEC();				// Set Carry Flag
void NES_OP_SED();				// Set Decimal Mode
void NES_OP_SEI();				// Set Interrupt Disable Status
void NES_OP_STA();	// Store Accumulator in Memory
void NES_OP_STX();	// Store Index X in Memory
void NES_OP_STY();	// Store Index Y in Memory
void NES_OP_TAX();				// Transfer Accumulator to Index X
void NES_OP_TAY();				// Transfer Accumulator to Index Y
void NES_OP_TSX();				// Transfer Stack Pointer to Index X
void NES_OP_TXA();				// Transfer Index X to Accumulator
void NES_OP_TXS();				// Transfer Index X to Stack Pointer
void NES_OP_TYA();				// Transfer Index Y to Accumulator

const string OP_NAME[] = {
	"BRK", "ORA", "...", "...", "...", "ORA", "ASL", "...", "PHP", "ORA", "ASL", "...", "...", "ORA", "ASL", "...", // 0
	"BPL", "ORA", "...", "...", "...", "ORA", "ASL", "...", "CLC", "ORA", "...", "...", "...", "ORA", "ASL", "...", // 1
	"JSR", "AND", "...", "...", "BIT", "AND", "ROL", "...", "PLP", "AND", "ROL", "...", "BIT", "AND", "ROL", "...", // 2
	"BMI", "AND", "...", "...", "...", "AND", "ROL", "...", "SEC", "AND", "...", "...", "...", "AND", "ROL", "...", // 3
	"RTI", "EOR", "...", "...", "...", "EOR", "LSR", "...", "PHA", "EOR", "LSR", "...", "JMP", "EOR", "LSR", "...", // 4
	"BVC", "EOR", "...", "...", "...", "EOR", "LSR", "...", "CLI", "EOR", "...", "...", "...", "EOR", "LSR", "...", // 5
	"RTS", "ADC", "...", "...", "...", "ADC", "ROR", "...", "PLA", "ADC", "ROR", "...", "JMP", "ADC", "ROR", "...", // 6
	"BVS", "ADC", "...", "...", "...", "ADC", "ROR", "...", "SEI", "ADC", "...", "...", "...", "ADC", "ROR", "...", // 7
	"...", "STA", "...", "...", "STY", "STA", "STX", "...", "DEY", "STA", "TSX", "...", "STY", "STA", "STX", "...", // 8
	"BCC", "STA", "...", "...", "STY", "STA", "STX", "...", "TYA", "STA", "TXS", "...", "...", "STA", "...", "...", // 9
	"LDY", "LDA", "LDX", "...", "LDY", "LDA", "LDX", "...", "TAY", "LDA", "TAX", "...", "LDY", "LDA", "LDX", "...", // A
	"BCS", "LDA", "...", "...", "LDY", "LDA", "LDX", "...", "CLV", "LDA", "...", "...", "LDY", "LDA", "LDX", "...", // B
	"CPY", "CMP", "...", "...", "CPY", "CMP", "DEC", "...", "INY", "CMP", "DEX", "...", "CPY", "CMP", "DEC", "...", // C
	"BNE", "CMP", "...", "...", "...", "CMP", "DEC", "...", "CLD", "CMP", "...", "...", "...", "CMP", "DEC", "...", // D
	"CPX", "SBC", "...", "...", "CPX", "SBC", "INC", "...", "INX", "SBC", "NOP", "...", "CPX", "SBC", "INC", "...", // E
	"BEQ", "SBC", "...", "...", "...", "SBC", "INC", "...", "SED", "SBC", "...", "...", "...", "SBC", "INC", "..." };

/* CPU REGISTER/VARIABLE DECLARATIONS */
byte A, X, Y;
/* 0. carry, 1. zero, 2. decimal [unavail], 3. interrupt disable, 
   4. breakpoint, 5. overflow, 6. negative sign*/
bitset<8> S;
byte SP;
word PC;

byte memory[0xFFFF];
byte* stack;

byte OP_CYCLES[0xFF];
void (*ADDR_MODE[0xFF])();
void (*OP_INSTR[0xFF])();

/* ADDRESSING MODE ACCESSERS */
byte OPCODE;
byte* OPERAND_ADDR;
word OPERAND_VAL;

int clockTicks = 0;

/* ADDRESSING MODES */
void NES_ADDR_implied()
{
	// do nothing
	OPERAND_ADDR = &OPERAND_VAL; // to stop debug messages from crashing
}
void NES_ADDR_immediate()
{
	OPERAND_VAL = memory[PC];
	OPERAND_ADDR = &OPERAND_VAL;
	PC++;
}
void NES_ADDR_absolute()
{
	OPERAND_ADDR = &memory[memory[PC] + (memory[PC + 1] << 8)];
	PC += 2;
}
void NES_ADDR_relative() // fix
{
	OPERAND_VAL = memory[PC];
	if (OPERAND_VAL & 0x80)
		OPERAND_VAL -= (byte)0x100;
	if (OPERAND_VAL >> 8 != PC >> 8)
		clockTicks++;
	OPERAND_ADDR = &OPERAND_VAL;
	PC++;
}
void NES_ADDR_indirect()
{
	word addr = memory[PC] + (memory[PC + 1] << 8);
	cout << (int)memory[addr] << " " << (int)memory[addr + 1];
	OPERAND_VAL = memory[addr] + (memory[addr + 1] << 8);
	OPERAND_ADDR = &OPERAND_VAL;
	PC += 2;
}
void NES_ADDR_absx()
{
	OPERAND_VAL = memory[PC] + (memory[PC + 1] << 8);
	if (OP_CYCLES[OPCODE] == 4)
		if (OPERAND_VAL >> 8 != (OPERAND_VAL + X) >> 8)
			clockTicks++;
	OPERAND_VAL += X;
	OPERAND_ADDR = &memory[OPERAND_VAL];
	PC += 2;
}
void NES_ADDR_absy()
{
	OPERAND_VAL = memory[PC] + (memory[PC + 1] << 8);
	if (OP_CYCLES[OPCODE] == 4)
		if (OPERAND_VAL >> 8 != (OPERAND_VAL + Y) >> 8)
			clockTicks++;
	OPERAND_VAL += Y;
	OPERAND_ADDR = &memory[OPERAND_VAL];
	PC += 2;
}
void NES_ADDR_zp()
{
	OPERAND_ADDR = &memory[memory[PC]];
	PC++;
}
void NES_ADDR_zpx()
{
	OPERAND_ADDR = &memory[(memory[PC] + X) & 0x00FF];
	PC++;
}
void NES_ADDR_zpy()
{
	OPERAND_ADDR = &memory[(memory[PC] + Y) & 0x00FF];
	PC++;
}
void NES_ADDR_indx()
{
	word addr = (memory[PC] + X) & 0x00FF;
	OPERAND_ADDR = &memory[memory[addr] + (memory[addr + 1] << 8)];
	PC++;
}
void NES_ADDR_indy()
{
	word addr = (memory[PC] + Y) & 0x00FF;
	OPERAND_ADDR = &memory[memory[addr] + (memory[addr + 1] << 8)];
	if (OP_CYCLES[OPCODE] == 5)
		if ((byte)OPERAND_ADDR >> 8 != ((byte)OPERAND_ADDR + Y) >> 8)
			clockTicks++;
	PC++;
}
void NES_ADDR_accum()
{
	OPERAND_ADDR = &A;
}

/* OPCODE DEFINITIONS */
void NES_OP_ADC()
{
	A += *OPERAND_ADDR + S[FLAG_CARRY];
	S[FLAG_CARRY] = A > 0xFF;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_OVERFLOW] = A > 0x7F;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Add Memory to Accumulator with Carry
void NES_OP_AND()
{
	A &= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// "AND" Memory with Accumulator
void NES_OP_ASL()
{
	S[FLAG_CARRY] = bitset<8>(A)[7];
	*OPERAND_ADDR <<= 1;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Shift Left One Bit (Memory or Accumulator)

void NES_OP_BCC()
{
	if (!S[FLAG_CARRY])
		PC += *OPERAND_ADDR;
}// Branch on Carry Clear
void NES_OP_BCS()
{
	if (S[FLAG_CARRY])
		PC += *OPERAND_ADDR;
}// Branch on Carry Set
void NES_OP_BEQ()
{
	if (S[FLAG_ZERO])
		PC += *OPERAND_ADDR;
}// Branch on Result Zero
void NES_OP_BIT()
{
	// TODO
}// Test Bits in Memory with Accumulator
void NES_OP_BMI()
{
	if (S[FLAG_NEGATIVE_SIGN])
		PC += *OPERAND_ADDR;
}// Branch on Result Minus
void NES_OP_BNE()
{
	if (!S[FLAG_ZERO])
		PC += *OPERAND_ADDR;
}// Branch on Result not Zero
void NES_OP_BPL()
{
	if (!S[FLAG_NEGATIVE_SIGN])
		PC += *OPERAND_ADDR;
}// Branch on Result Plus
void NES_OP_BRK()
{
	S[FLAG_BREAKPOINT] = 1;
	PC++;
	stack[SP--] = PC >> 4;
	stack[SP--] = PC & 0xFF;
	stack[SP--] = (byte)S.to_ulong();
	PC = memory[0xFFFE] | (memory[0xFFFF] << 8);
}// Force Break
void NES_OP_BVC()
{
	if (!S[FLAG_OVERFLOW])
		PC += *OPERAND_ADDR;
}// Branch on Overflow Clear
void NES_OP_BVS()
{
	if (S[FLAG_OVERFLOW])
		PC += *OPERAND_ADDR;
}// Branch on Overflow Set

void NES_OP_CLC()
{
	S[FLAG_CARRY] = 0;
}// Clear Carry Flag
void NES_OP_CLD()
{
	S[FLAG_DECIMAL] = 0;
}// Clear Decimal Mode
void NES_OP_CLI()
{
	S[FLAG_INTERRUPT_DISABLE] = 0;
}// Clear interrupt Disable Bit
void NES_OP_CLV()
{
	S[FLAG_OVERFLOW] = 0;
}// Clear Overflow Flag
void NES_OP_CMP()
{
	S[FLAG_CARRY] = A >= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == *OPERAND_ADDR;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A - *OPERAND_ADDR)[7];
}// Compare Memory and Accumulator
void NES_OP_CPX()
{
	S[FLAG_CARRY] = X >= *OPERAND_ADDR;
	S[FLAG_ZERO] = X == *OPERAND_ADDR;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X - *OPERAND_ADDR)[7];
}// Compare Memory and Index X
void NES_OP_CPY()
{
	S[FLAG_CARRY] = Y >= *OPERAND_ADDR;
	S[FLAG_ZERO] = Y == *OPERAND_ADDR;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y - *OPERAND_ADDR)[7];
}// Compare Memory and Index Y

void NES_OP_DEC()
{
	--*OPERAND_ADDR;
	S[FLAG_ZERO] = *OPERAND_ADDR == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(*OPERAND_ADDR)[7];
}// Decrement Memory by One
void NES_OP_DEX()
{
	--X;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Decrement Index X by One
void NES_OP_DEY()
{
	--Y;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Decrement Index Y by One
void NES_OP_EOR()
{
	A ^= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// "Exclusive-Or" Memory with Accumulator

void NES_OP_INC()
{
	++*OPERAND_ADDR;
	S[FLAG_ZERO] = *OPERAND_ADDR == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(*OPERAND_ADDR)[7];
}// Increment Memory by One
void NES_OP_INX()
{
	++X;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Increment Index X by One
void NES_OP_INY()
{
	++Y;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Increment Index Y by One

void NES_OP_JMP()
{
	PC = *OPERAND_ADDR;
}// Jump to New Location
void NES_OP_JSR()
{
	stack[SP--] = PC >> 4;
	stack[SP--] = PC & 0xFF;
	PC = *OPERAND_ADDR;
}// Jump to New Location Saving Return Address

void NES_OP_LDA()
{
	A = *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Load Accumulator with Memory
void NES_OP_LDX()
{
	X = *OPERAND_ADDR;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Load Index X with Memory
void NES_OP_LDY()
{
	Y = *OPERAND_ADDR;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Load Index Y with Memory 
void NES_OP_LSR()
{
	S[FLAG_CARRY] = bitset<8>(*OPERAND_ADDR)[0];
	*OPERAND_ADDR >>= 1;
	S[FLAG_ZERO] = *OPERAND_ADDR == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(*OPERAND_ADDR)[7];
}// Shift Right One Bit(Memory or Accumulator)

void NES_OP_NOP()
{
	// this is a waste of code
}// No Operation

void NES_OP_ORA()
{
	A |= *OPERAND_ADDR;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// "OR" Memory with Accumulator

void NES_OP_PHA()
{
	stack[SP--] = A;
}// Push Accumulator on Stack
void NES_OP_PHP()
{
	stack[SP--] = (byte)S.to_ulong();
}// Push Processor Status on Stack
void NES_OP_PLA()
{
	A = stack[++SP];
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Pull Accumulator from Stack
void NES_OP_PLP()
{
	S = stack[++SP];
}// Pull Processor Status from Stack

void NES_OP_ROL()
{
	bitset<8> tmp(*OPERAND_ADDR);
	bool old = tmp[7];
	tmp <<= 1;
	tmp[0] = S[FLAG_CARRY];
	S[FLAG_CARRY] = old;
	*OPERAND_ADDR = (byte)tmp.to_ulong();
}// Rotate One Bit Left(Memory or Accumulator)
void NES_OP_ROR()
{
	bitset<8> tmp(*OPERAND_ADDR);
	bool old = tmp[0];
	tmp >>= 1;
	tmp[7] = S[FLAG_CARRY];
	S[FLAG_CARRY] = old;
	*OPERAND_ADDR = (byte)tmp.to_ulong();
}// Rotate One Bit Right(Memory or Accumulator)
void NES_OP_RTI()
{
	NES_OP_PLP();
	NES_OP_RTS();
}// Return from Interrupt
void NES_OP_RTS()
{
	PC = stack[++SP] | (stack[++SP] << 8);
}// Return from Subroutine

void NES_OP_SBC()
{
	A -= *OPERAND_ADDR - !S[FLAG_CARRY];
	S[FLAG_CARRY] = !bitset<8>(A)[7];
	S[FLAG_ZERO] = A == 0;
	S[FLAG_OVERFLOW] = A > 0x7F;
	S[FLAG_NEGATIVE_SIGN] = !S[FLAG_CARRY];
}// Subtract Memory from Accumulator with Borrow
void NES_OP_SEC()
{
	S[FLAG_CARRY] = 1;
}// Set Carry Flag
void NES_OP_SED()
{
	S[FLAG_DECIMAL] = 1;
}// Set Decimal Mode
void NES_OP_SEI()
{
	S[FLAG_INTERRUPT_DISABLE] = 1;
}// Set Interrupt Disable Status
void NES_OP_STA()
{
	*OPERAND_ADDR = A;
}// Store Accumulator in Memory
void NES_OP_STX()
{
	*OPERAND_ADDR = X;
}// Store Index X in Memory
void NES_OP_STY()
{
	*OPERAND_ADDR = Y;
}// Store Index Y in Memory

void NES_OP_TAX()
{
	X = A;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Transfer Accumulator to Index X
void NES_OP_TAY()
{
	Y = A;
	S[FLAG_ZERO] = Y == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(Y)[7];
}// Transfer Accumulator to Index Y
void NES_OP_TSX()
{
	X = SP;
	S[FLAG_ZERO] = X == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(X)[7];
}// Transfer Stack Pointer to Index X
void NES_OP_TXA()
{
	A = X;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Transfer Index X to Accumulator
void NES_OP_TXS()
{
	SP = X;
}// Transfer Index X to Stack Pointer
void NES_OP_TYA()
{
	A = Y;
	S[FLAG_ZERO] = A == 0;
	S[FLAG_NEGATIVE_SIGN] = bitset<8>(A)[7];
}// Transfer Index Y to Accumulator

void NES_init()
{
	A, X, Y, S = 0;
	PC = 0x0600; // reset vector! memory[0xFFFC] | (memory[0xFFFD] << 8)
	SP = 0x00FF;
	stack = memory + 0x0100;
	printf("NES EMULATOR (6502) by GORDON GUAN\n");
	printf("REGISTER MAPPING:\n");
	printf("A:0x%X; X:0x%X; Y:0x%X; S:0x%X\nSP:0x%X; PC:0x%X; OP_VAL:0x%X\n\n", 
		&A, &X, &Y, &S, &SP, &PC, OPERAND_VAL);
	printf(":: INSTRUCTION EXECUTION\n");
	/* DECLARE OPCODE PROPS */
	OP_CYCLES[0x00] = 7; OP_INSTR[0x00] = NES_OP_BRK; ADDR_MODE[0x00] = NES_ADDR_implied;
	OP_CYCLES[0x01] = 6; OP_INSTR[0x01] = NES_OP_ORA; ADDR_MODE[0x01] = NES_ADDR_indx;
	OP_CYCLES[0x02] = 2; OP_INSTR[0x02] = NES_OP_NOP; ADDR_MODE[0x02] = NES_ADDR_implied;
	OP_CYCLES[0x03] = 2; OP_INSTR[0x03] = NES_OP_NOP; ADDR_MODE[0x03] = NES_ADDR_implied;
	OP_CYCLES[0x04] = 3; OP_INSTR[0x04] = NES_OP_NOP; ADDR_MODE[0x04] = NES_ADDR_implied;
	OP_CYCLES[0x05] = 3; OP_INSTR[0x05] = NES_OP_ORA; ADDR_MODE[0x05] = NES_ADDR_zp;
	OP_CYCLES[0x06] = 5; OP_INSTR[0x06] = NES_OP_ASL; ADDR_MODE[0x06] = NES_ADDR_zp;
	OP_CYCLES[0x07] = 2; OP_INSTR[0x07] = NES_OP_NOP; ADDR_MODE[0x07] = NES_ADDR_implied;
	OP_CYCLES[0x08] = 3; OP_INSTR[0x08] = NES_OP_PHP; ADDR_MODE[0x08] = NES_ADDR_implied;
	OP_CYCLES[0x09] = 3; OP_INSTR[0x09] = NES_OP_ORA; ADDR_MODE[0x09] = NES_ADDR_immediate;
	OP_CYCLES[0x0a] = 2; OP_INSTR[0x0a] = NES_OP_ASL; ADDR_MODE[0x0a] = NES_ADDR_accum;
	OP_CYCLES[0x0b] = 2; OP_INSTR[0x0b] = NES_OP_NOP; ADDR_MODE[0x0b] = NES_ADDR_implied;
	OP_CYCLES[0x0c] = 4; OP_INSTR[0x0c] = NES_OP_NOP; ADDR_MODE[0x0c] = NES_ADDR_implied;
	OP_CYCLES[0x0d] = 4; OP_INSTR[0x0d] = NES_OP_ORA; ADDR_MODE[0x0d] = NES_ADDR_absolute;
	OP_CYCLES[0x0e] = 6; OP_INSTR[0x0e] = NES_OP_ASL; ADDR_MODE[0x0e] = NES_ADDR_absolute;
	OP_CYCLES[0x0f] = 2; OP_INSTR[0x0f] = NES_OP_NOP; ADDR_MODE[0x0f] = NES_ADDR_implied;
	OP_CYCLES[0x10] = 2; OP_INSTR[0x10] = NES_OP_BPL; ADDR_MODE[0x10] = NES_ADDR_relative;
	OP_CYCLES[0x11] = 5; OP_INSTR[0x11] = NES_OP_ORA; ADDR_MODE[0x11] = NES_ADDR_indy;
	OP_CYCLES[0x12] = 3; OP_INSTR[0x12] = NES_OP_NOP; ADDR_MODE[0x12] = NES_ADDR_implied;
	OP_CYCLES[0x13] = 2; OP_INSTR[0x13] = NES_OP_NOP; ADDR_MODE[0x13] = NES_ADDR_implied;
	OP_CYCLES[0x14] = 3; OP_INSTR[0x14] = NES_OP_NOP; ADDR_MODE[0x14] = NES_ADDR_implied;
	OP_CYCLES[0x15] = 4; OP_INSTR[0x15] = NES_OP_ORA; ADDR_MODE[0x15] = NES_ADDR_zpx;
	OP_CYCLES[0x16] = 6; OP_INSTR[0x16] = NES_OP_ASL; ADDR_MODE[0x16] = NES_ADDR_zpx;
	OP_CYCLES[0x17] = 2; OP_INSTR[0x17] = NES_OP_NOP; ADDR_MODE[0x17] = NES_ADDR_implied;
	OP_CYCLES[0x18] = 2; OP_INSTR[0x18] = NES_OP_CLC; ADDR_MODE[0x18] = NES_ADDR_implied;
	OP_CYCLES[0x19] = 4; OP_INSTR[0x19] = NES_OP_ORA; ADDR_MODE[0x19] = NES_ADDR_absy;
	OP_CYCLES[0x1a] = 2; OP_INSTR[0x1a] = NES_OP_NOP; ADDR_MODE[0x1a] = NES_ADDR_implied;
	OP_CYCLES[0x1b] = 2; OP_INSTR[0x1b] = NES_OP_NOP; ADDR_MODE[0x1b] = NES_ADDR_implied;
	OP_CYCLES[0x1c] = 4; OP_INSTR[0x1c] = NES_OP_NOP; ADDR_MODE[0x1c] = NES_ADDR_implied;
	OP_CYCLES[0x1d] = 4; OP_INSTR[0x1d] = NES_OP_ORA; ADDR_MODE[0x1d] = NES_ADDR_absx;
	OP_CYCLES[0x1e] = 7; OP_INSTR[0x1e] = NES_OP_ASL; ADDR_MODE[0x1e] = NES_ADDR_absx;
	OP_CYCLES[0x1f] = 2; OP_INSTR[0x1f] = NES_OP_NOP; ADDR_MODE[0x1f] = NES_ADDR_implied;
	OP_CYCLES[0x20] = 6; OP_INSTR[0x20] = NES_OP_JSR; ADDR_MODE[0x20] = NES_ADDR_absolute;
	OP_CYCLES[0x21] = 6; OP_INSTR[0x21] = NES_OP_AND; ADDR_MODE[0x21] = NES_ADDR_indx;
	OP_CYCLES[0x22] = 2; OP_INSTR[0x22] = NES_OP_NOP; ADDR_MODE[0x22] = NES_ADDR_implied;
	OP_CYCLES[0x23] = 2; OP_INSTR[0x23] = NES_OP_NOP; ADDR_MODE[0x23] = NES_ADDR_implied;
	OP_CYCLES[0x24] = 3; OP_INSTR[0x24] = NES_OP_BIT; ADDR_MODE[0x24] = NES_ADDR_zp;
	OP_CYCLES[0x25] = 3; OP_INSTR[0x25] = NES_OP_AND; ADDR_MODE[0x25] = NES_ADDR_zp;
	OP_CYCLES[0x26] = 5; OP_INSTR[0x26] = NES_OP_ROL; ADDR_MODE[0x26] = NES_ADDR_zp;
	OP_CYCLES[0x27] = 2; OP_INSTR[0x27] = NES_OP_NOP; ADDR_MODE[0x27] = NES_ADDR_implied;
	OP_CYCLES[0x28] = 4; OP_INSTR[0x28] = NES_OP_PLP; ADDR_MODE[0x28] = NES_ADDR_implied;
	OP_CYCLES[0x29] = 3; OP_INSTR[0x29] = NES_OP_AND; ADDR_MODE[0x29] = NES_ADDR_immediate;
	OP_CYCLES[0x2a] = 2; OP_INSTR[0x2a] = NES_OP_ROL; ADDR_MODE[0x2a] = NES_ADDR_accum;
	OP_CYCLES[0x2b] = 2; OP_INSTR[0x2b] = NES_OP_NOP; ADDR_MODE[0x2b] = NES_ADDR_implied;
	OP_CYCLES[0x2c] = 4; OP_INSTR[0x2c] = NES_OP_BIT; ADDR_MODE[0x2c] = NES_ADDR_absolute;
	OP_CYCLES[0x2d] = 4; OP_INSTR[0x2d] = NES_OP_AND; ADDR_MODE[0x2d] = NES_ADDR_absolute;
	OP_CYCLES[0x2e] = 6; OP_INSTR[0x2e] = NES_OP_ROL; ADDR_MODE[0x2e] = NES_ADDR_absolute;
	OP_CYCLES[0x2f] = 2; OP_INSTR[0x2f] = NES_OP_NOP; ADDR_MODE[0x2f] = NES_ADDR_implied;
	OP_CYCLES[0x30] = 2; OP_INSTR[0x30] = NES_OP_BMI; ADDR_MODE[0x30] = NES_ADDR_relative;
	OP_CYCLES[0x31] = 5; OP_INSTR[0x31] = NES_OP_AND; ADDR_MODE[0x31] = NES_ADDR_indy;
	OP_CYCLES[0x32] = 3; OP_INSTR[0x32] = NES_OP_NOP; ADDR_MODE[0x32] = NES_ADDR_implied;
	OP_CYCLES[0x33] = 2; OP_INSTR[0x33] = NES_OP_NOP; ADDR_MODE[0x33] = NES_ADDR_implied;
	OP_CYCLES[0x34] = 4; OP_INSTR[0x34] = NES_OP_NOP; ADDR_MODE[0x34] = NES_ADDR_implied;
	OP_CYCLES[0x35] = 4; OP_INSTR[0x35] = NES_OP_AND; ADDR_MODE[0x35] = NES_ADDR_zpx;
	OP_CYCLES[0x36] = 6; OP_INSTR[0x36] = NES_OP_ROL; ADDR_MODE[0x36] = NES_ADDR_zpx;
	OP_CYCLES[0x37] = 2; OP_INSTR[0x37] = NES_OP_NOP; ADDR_MODE[0x37] = NES_ADDR_implied;
	OP_CYCLES[0x38] = 2; OP_INSTR[0x38] = NES_OP_SEC; ADDR_MODE[0x38] = NES_ADDR_implied;
	OP_CYCLES[0x39] = 4; OP_INSTR[0x39] = NES_OP_AND; ADDR_MODE[0x39] = NES_ADDR_absy;
	OP_CYCLES[0x3a] = 2; OP_INSTR[0x3a] = NES_OP_NOP; ADDR_MODE[0x3a] = NES_ADDR_implied;
	OP_CYCLES[0x3b] = 2; OP_INSTR[0x3b] = NES_OP_NOP; ADDR_MODE[0x3b] = NES_ADDR_implied;
	OP_CYCLES[0x3c] = 4; OP_INSTR[0x3c] = NES_OP_NOP; ADDR_MODE[0x3c] = NES_ADDR_implied;
	OP_CYCLES[0x3d] = 4; OP_INSTR[0x3d] = NES_OP_AND; ADDR_MODE[0x3d] = NES_ADDR_absx;
	OP_CYCLES[0x3e] = 7; OP_INSTR[0x3e] = NES_OP_ROL; ADDR_MODE[0x3e] = NES_ADDR_absx;
	OP_CYCLES[0x3f] = 2; OP_INSTR[0x3f] = NES_OP_NOP; ADDR_MODE[0x3f] = NES_ADDR_implied;
	OP_CYCLES[0x40] = 6; OP_INSTR[0x40] = NES_OP_RTI; ADDR_MODE[0x40] = NES_ADDR_implied;
	OP_CYCLES[0x41] = 6; OP_INSTR[0x41] = NES_OP_EOR; ADDR_MODE[0x41] = NES_ADDR_indx;
	OP_CYCLES[0x42] = 2; OP_INSTR[0x42] = NES_OP_NOP; ADDR_MODE[0x42] = NES_ADDR_implied;
	OP_CYCLES[0x43] = 2; OP_INSTR[0x43] = NES_OP_NOP; ADDR_MODE[0x43] = NES_ADDR_implied;
	OP_CYCLES[0x44] = 2; OP_INSTR[0x44] = NES_OP_NOP; ADDR_MODE[0x44] = NES_ADDR_implied;
	OP_CYCLES[0x45] = 3; OP_INSTR[0x45] = NES_OP_EOR; ADDR_MODE[0x45] = NES_ADDR_zp;
	OP_CYCLES[0x46] = 5; OP_INSTR[0x46] = NES_OP_LSR; ADDR_MODE[0x46] = NES_ADDR_zp;
	OP_CYCLES[0x47] = 2; OP_INSTR[0x47] = NES_OP_NOP; ADDR_MODE[0x47] = NES_ADDR_implied;
	OP_CYCLES[0x48] = 3; OP_INSTR[0x48] = NES_OP_PHA; ADDR_MODE[0x48] = NES_ADDR_implied;
	OP_CYCLES[0x49] = 3; OP_INSTR[0x49] = NES_OP_EOR; ADDR_MODE[0x49] = NES_ADDR_immediate;
	OP_CYCLES[0x4a] = 2; OP_INSTR[0x4a] = NES_OP_LSR; ADDR_MODE[0x4a] = NES_ADDR_accum;
	OP_CYCLES[0x4b] = 2; OP_INSTR[0x4b] = NES_OP_NOP; ADDR_MODE[0x4b] = NES_ADDR_implied;
	OP_CYCLES[0x4c] = 3; OP_INSTR[0x4c] = NES_OP_JMP; ADDR_MODE[0x4c] = NES_ADDR_absolute;
	OP_CYCLES[0x4d] = 4; OP_INSTR[0x4d] = NES_OP_EOR; ADDR_MODE[0x4d] = NES_ADDR_absolute;
	OP_CYCLES[0x4e] = 6; OP_INSTR[0x4e] = NES_OP_LSR; ADDR_MODE[0x4e] = NES_ADDR_absolute;
	OP_CYCLES[0x4f] = 2; OP_INSTR[0x4f] = NES_OP_NOP; ADDR_MODE[0x4f] = NES_ADDR_implied;
	OP_CYCLES[0x50] = 2; OP_INSTR[0x50] = NES_OP_BVC; ADDR_MODE[0x50] = NES_ADDR_relative;
	OP_CYCLES[0x51] = 5; OP_INSTR[0x51] = NES_OP_EOR; ADDR_MODE[0x51] = NES_ADDR_indy;
	OP_CYCLES[0x52] = 3; OP_INSTR[0x52] = NES_OP_NOP; ADDR_MODE[0x52] = NES_ADDR_implied;
	OP_CYCLES[0x53] = 2; OP_INSTR[0x53] = NES_OP_NOP; ADDR_MODE[0x53] = NES_ADDR_implied;
	OP_CYCLES[0x54] = 2; OP_INSTR[0x54] = NES_OP_NOP; ADDR_MODE[0x54] = NES_ADDR_implied;
	OP_CYCLES[0x55] = 4; OP_INSTR[0x55] = NES_OP_EOR; ADDR_MODE[0x55] = NES_ADDR_zpx;
	OP_CYCLES[0x56] = 6; OP_INSTR[0x56] = NES_OP_LSR; ADDR_MODE[0x56] = NES_ADDR_zpx;
	OP_CYCLES[0x57] = 2; OP_INSTR[0x57] = NES_OP_NOP; ADDR_MODE[0x57] = NES_ADDR_implied;
	OP_CYCLES[0x58] = 2; OP_INSTR[0x58] = NES_OP_CLI; ADDR_MODE[0x58] = NES_ADDR_implied;
	OP_CYCLES[0x59] = 4; OP_INSTR[0x59] = NES_OP_EOR; ADDR_MODE[0x59] = NES_ADDR_absy;
	OP_CYCLES[0x5a] = 3; OP_INSTR[0x5a] = NES_OP_NOP; ADDR_MODE[0x5a] = NES_ADDR_implied;
	OP_CYCLES[0x5b] = 2; OP_INSTR[0x5b] = NES_OP_NOP; ADDR_MODE[0x5b] = NES_ADDR_implied;
	OP_CYCLES[0x5c] = 2; OP_INSTR[0x5c] = NES_OP_NOP; ADDR_MODE[0x5c] = NES_ADDR_implied;
	OP_CYCLES[0x5d] = 4; OP_INSTR[0x5d] = NES_OP_EOR; ADDR_MODE[0x5d] = NES_ADDR_absx;
	OP_CYCLES[0x5e] = 7; OP_INSTR[0x5e] = NES_OP_LSR; ADDR_MODE[0x5e] = NES_ADDR_absx;
	OP_CYCLES[0x5f] = 2; OP_INSTR[0x5f] = NES_OP_NOP; ADDR_MODE[0x5f] = NES_ADDR_implied;
	OP_CYCLES[0x60] = 6; OP_INSTR[0x60] = NES_OP_RTS; ADDR_MODE[0x60] = NES_ADDR_implied;
	OP_CYCLES[0x61] = 6; OP_INSTR[0x61] = NES_OP_ADC; ADDR_MODE[0x61] = NES_ADDR_indx;
	OP_CYCLES[0x62] = 2; OP_INSTR[0x62] = NES_OP_NOP; ADDR_MODE[0x62] = NES_ADDR_implied;
	OP_CYCLES[0x63] = 2; OP_INSTR[0x63] = NES_OP_NOP; ADDR_MODE[0x63] = NES_ADDR_implied;
	OP_CYCLES[0x64] = 3; OP_INSTR[0x64] = NES_OP_NOP; ADDR_MODE[0x64] = NES_ADDR_implied;
	OP_CYCLES[0x65] = 3; OP_INSTR[0x65] = NES_OP_ADC; ADDR_MODE[0x65] = NES_ADDR_zp;
	OP_CYCLES[0x66] = 5; OP_INSTR[0x66] = NES_OP_ROR; ADDR_MODE[0x66] = NES_ADDR_zp;
	OP_CYCLES[0x67] = 2; OP_INSTR[0x67] = NES_OP_NOP; ADDR_MODE[0x67] = NES_ADDR_implied;
	OP_CYCLES[0x68] = 4; OP_INSTR[0x68] = NES_OP_PLA; ADDR_MODE[0x68] = NES_ADDR_implied;
	OP_CYCLES[0x69] = 3; OP_INSTR[0x69] = NES_OP_ADC; ADDR_MODE[0x69] = NES_ADDR_immediate;
	OP_CYCLES[0x6a] = 2; OP_INSTR[0x6a] = NES_OP_ROR; ADDR_MODE[0x6a] = NES_ADDR_accum;
	OP_CYCLES[0x6b] = 2; OP_INSTR[0x6b] = NES_OP_NOP; ADDR_MODE[0x6b] = NES_ADDR_implied;
	OP_CYCLES[0x6c] = 5; OP_INSTR[0x6c] = NES_OP_JMP; ADDR_MODE[0x6c] = NES_ADDR_indirect;
	OP_CYCLES[0x6d] = 4; OP_INSTR[0x6d] = NES_OP_ADC; ADDR_MODE[0x6d] = NES_ADDR_absolute;
	OP_CYCLES[0x6e] = 6; OP_INSTR[0x6e] = NES_OP_ROR; ADDR_MODE[0x6e] = NES_ADDR_absolute;
	OP_CYCLES[0x6f] = 2; OP_INSTR[0x6f] = NES_OP_NOP; ADDR_MODE[0x6f] = NES_ADDR_implied;
	OP_CYCLES[0x70] = 2; OP_INSTR[0x70] = NES_OP_BVS; ADDR_MODE[0x70] = NES_ADDR_relative;
	OP_CYCLES[0x71] = 5; OP_INSTR[0x71] = NES_OP_ADC; ADDR_MODE[0x71] = NES_ADDR_indy;
	OP_CYCLES[0x72] = 3; OP_INSTR[0x72] = NES_OP_NOP; ADDR_MODE[0x72] = NES_ADDR_implied;
	OP_CYCLES[0x73] = 2; OP_INSTR[0x73] = NES_OP_NOP; ADDR_MODE[0x73] = NES_ADDR_implied;
	OP_CYCLES[0x74] = 4; OP_INSTR[0x74] = NES_OP_NOP; ADDR_MODE[0x74] = NES_ADDR_implied;
	OP_CYCLES[0x75] = 4; OP_INSTR[0x75] = NES_OP_ADC; ADDR_MODE[0x75] = NES_ADDR_zpx;
	OP_CYCLES[0x76] = 6; OP_INSTR[0x76] = NES_OP_ROR; ADDR_MODE[0x76] = NES_ADDR_zpx;
	OP_CYCLES[0x77] = 2; OP_INSTR[0x77] = NES_OP_NOP; ADDR_MODE[0x77] = NES_ADDR_implied;
	OP_CYCLES[0x78] = 2; OP_INSTR[0x78] = NES_OP_SEI; ADDR_MODE[0x78] = NES_ADDR_implied;
	OP_CYCLES[0x79] = 4; OP_INSTR[0x79] = NES_OP_ADC; ADDR_MODE[0x79] = NES_ADDR_absy;
	OP_CYCLES[0x7a] = 4; OP_INSTR[0x7a] = NES_OP_NOP; ADDR_MODE[0x7a] = NES_ADDR_implied;
	OP_CYCLES[0x7b] = 2; OP_INSTR[0x7b] = NES_OP_NOP; ADDR_MODE[0x7b] = NES_ADDR_implied;
	OP_CYCLES[0x7c] = 6; OP_INSTR[0x7c] = NES_OP_NOP; ADDR_MODE[0x7c] = NES_ADDR_implied;
	OP_CYCLES[0x7d] = 4; OP_INSTR[0x7d] = NES_OP_ADC; ADDR_MODE[0x7d] = NES_ADDR_absx;
	OP_CYCLES[0x7e] = 7; OP_INSTR[0x7e] = NES_OP_ROR; ADDR_MODE[0x7e] = NES_ADDR_absx;
	OP_CYCLES[0x7f] = 2; OP_INSTR[0x7f] = NES_OP_NOP; ADDR_MODE[0x7f] = NES_ADDR_implied;
	OP_CYCLES[0x80] = 2; OP_INSTR[0x80] = NES_OP_NOP; ADDR_MODE[0x80] = NES_ADDR_implied;
	OP_CYCLES[0x81] = 6; OP_INSTR[0x81] = NES_OP_STA; ADDR_MODE[0x81] = NES_ADDR_indx;
	OP_CYCLES[0x82] = 2; OP_INSTR[0x82] = NES_OP_NOP; ADDR_MODE[0x82] = NES_ADDR_implied;
	OP_CYCLES[0x83] = 2; OP_INSTR[0x83] = NES_OP_NOP; ADDR_MODE[0x83] = NES_ADDR_implied;
	OP_CYCLES[0x84] = 2; OP_INSTR[0x84] = NES_OP_STY; ADDR_MODE[0x84] = NES_ADDR_zp;
	OP_CYCLES[0x85] = 2; OP_INSTR[0x85] = NES_OP_STA; ADDR_MODE[0x85] = NES_ADDR_zp;
	OP_CYCLES[0x86] = 2; OP_INSTR[0x86] = NES_OP_STX; ADDR_MODE[0x86] = NES_ADDR_zp;
	OP_CYCLES[0x87] = 2; OP_INSTR[0x87] = NES_OP_NOP; ADDR_MODE[0x87] = NES_ADDR_implied;
	OP_CYCLES[0x88] = 2; OP_INSTR[0x88] = NES_OP_DEY; ADDR_MODE[0x88] = NES_ADDR_implied;
	OP_CYCLES[0x89] = 2; OP_INSTR[0x89] = NES_OP_NOP; ADDR_MODE[0x89] = NES_ADDR_implied;
	OP_CYCLES[0x8a] = 2; OP_INSTR[0x8a] = NES_OP_TXA; ADDR_MODE[0x8a] = NES_ADDR_implied;
	OP_CYCLES[0x8b] = 2; OP_INSTR[0x8b] = NES_OP_NOP; ADDR_MODE[0x8b] = NES_ADDR_implied;
	OP_CYCLES[0x8c] = 4; OP_INSTR[0x8c] = NES_OP_STY; ADDR_MODE[0x8c] = NES_ADDR_absolute;
	OP_CYCLES[0x8d] = 4; OP_INSTR[0x8d] = NES_OP_STA; ADDR_MODE[0x8d] = NES_ADDR_absolute;
	OP_CYCLES[0x8e] = 4; OP_INSTR[0x8e] = NES_OP_STX; ADDR_MODE[0x8e] = NES_ADDR_absolute;
	OP_CYCLES[0x8f] = 2; OP_INSTR[0x8f] = NES_OP_NOP; ADDR_MODE[0x8f] = NES_ADDR_implied;
	OP_CYCLES[0x90] = 2; OP_INSTR[0x90] = NES_OP_BCC; ADDR_MODE[0x90] = NES_ADDR_relative;
	OP_CYCLES[0x91] = 6; OP_INSTR[0x91] = NES_OP_STA; ADDR_MODE[0x91] = NES_ADDR_indy;
	OP_CYCLES[0x92] = 3; OP_INSTR[0x92] = NES_OP_NOP; ADDR_MODE[0x92] = NES_ADDR_implied;
	OP_CYCLES[0x93] = 2; OP_INSTR[0x93] = NES_OP_NOP; ADDR_MODE[0x93] = NES_ADDR_implied;
	OP_CYCLES[0x94] = 4; OP_INSTR[0x94] = NES_OP_STY; ADDR_MODE[0x94] = NES_ADDR_zpx;
	OP_CYCLES[0x95] = 4; OP_INSTR[0x95] = NES_OP_STA; ADDR_MODE[0x95] = NES_ADDR_zpx;
	OP_CYCLES[0x96] = 4; OP_INSTR[0x96] = NES_OP_STX; ADDR_MODE[0x96] = NES_ADDR_zpy;
	OP_CYCLES[0x97] = 2; OP_INSTR[0x97] = NES_OP_NOP; ADDR_MODE[0x97] = NES_ADDR_implied;
	OP_CYCLES[0x98] = 2; OP_INSTR[0x98] = NES_OP_TYA; ADDR_MODE[0x98] = NES_ADDR_implied;
	OP_CYCLES[0x99] = 5; OP_INSTR[0x99] = NES_OP_STA; ADDR_MODE[0x99] = NES_ADDR_absy;
	OP_CYCLES[0x9a] = 2; OP_INSTR[0x9a] = NES_OP_TXS; ADDR_MODE[0x9a] = NES_ADDR_implied;
	OP_CYCLES[0x9b] = 2; OP_INSTR[0x9b] = NES_OP_NOP; ADDR_MODE[0x9b] = NES_ADDR_implied;
	OP_CYCLES[0x9c] = 4; OP_INSTR[0x9c] = NES_OP_NOP; ADDR_MODE[0x9c] = NES_ADDR_implied;
	OP_CYCLES[0x9d] = 5; OP_INSTR[0x9d] = NES_OP_STA; ADDR_MODE[0x9d] = NES_ADDR_absx;
	OP_CYCLES[0x9e] = 5; OP_INSTR[0x9e] = NES_OP_NOP; ADDR_MODE[0x9e] = NES_ADDR_implied;
	OP_CYCLES[0x9f] = 2; OP_INSTR[0x9f] = NES_OP_NOP; ADDR_MODE[0x9f] = NES_ADDR_implied;
	OP_CYCLES[0xa0] = 3; OP_INSTR[0xa0] = NES_OP_LDY; ADDR_MODE[0xa0] = NES_ADDR_immediate;
	OP_CYCLES[0xa1] = 6; OP_INSTR[0xa1] = NES_OP_LDA; ADDR_MODE[0xa1] = NES_ADDR_indx;
	OP_CYCLES[0xa2] = 3; OP_INSTR[0xa2] = NES_OP_LDX; ADDR_MODE[0xa2] = NES_ADDR_immediate;
	OP_CYCLES[0xa3] = 2; OP_INSTR[0xa3] = NES_OP_NOP; ADDR_MODE[0xa3] = NES_ADDR_implied;
	OP_CYCLES[0xa4] = 3; OP_INSTR[0xa4] = NES_OP_LDY; ADDR_MODE[0xa4] = NES_ADDR_zp;
	OP_CYCLES[0xa5] = 3; OP_INSTR[0xa5] = NES_OP_LDA; ADDR_MODE[0xa5] = NES_ADDR_zp;
	OP_CYCLES[0xa6] = 3; OP_INSTR[0xa6] = NES_OP_LDX; ADDR_MODE[0xa6] = NES_ADDR_zp;
	OP_CYCLES[0xa7] = 2; OP_INSTR[0xa7] = NES_OP_NOP; ADDR_MODE[0xa7] = NES_ADDR_implied;
	OP_CYCLES[0xa8] = 2; OP_INSTR[0xa8] = NES_OP_TAY; ADDR_MODE[0xa8] = NES_ADDR_implied;
	OP_CYCLES[0xa9] = 3; OP_INSTR[0xa9] = NES_OP_LDA; ADDR_MODE[0xa9] = NES_ADDR_immediate;
	OP_CYCLES[0xaa] = 2; OP_INSTR[0xaa] = NES_OP_TAX; ADDR_MODE[0xaa] = NES_ADDR_implied;
	OP_CYCLES[0xab] = 2; OP_INSTR[0xab] = NES_OP_NOP; ADDR_MODE[0xab] = NES_ADDR_implied;
	OP_CYCLES[0xac] = 4; OP_INSTR[0xac] = NES_OP_LDY; ADDR_MODE[0xac] = NES_ADDR_absolute;
	OP_CYCLES[0xad] = 4; OP_INSTR[0xad] = NES_OP_LDA; ADDR_MODE[0xad] = NES_ADDR_absolute;
	OP_CYCLES[0xae] = 4; OP_INSTR[0xae] = NES_OP_LDX; ADDR_MODE[0xae] = NES_ADDR_absolute;
	OP_CYCLES[0xaf] = 2; OP_INSTR[0xaf] = NES_OP_NOP; ADDR_MODE[0xaf] = NES_ADDR_implied;
	OP_CYCLES[0xb0] = 2; OP_INSTR[0xb0] = NES_OP_BCS; ADDR_MODE[0xb0] = NES_ADDR_relative;
	OP_CYCLES[0xb1] = 5; OP_INSTR[0xb1] = NES_OP_LDA; ADDR_MODE[0xb1] = NES_ADDR_indy;
	OP_CYCLES[0xb2] = 3; OP_INSTR[0xb2] = NES_OP_NOP; ADDR_MODE[0xb2] = NES_ADDR_implied;
	OP_CYCLES[0xb3] = 2; OP_INSTR[0xb3] = NES_OP_NOP; ADDR_MODE[0xb3] = NES_ADDR_implied;
	OP_CYCLES[0xb4] = 4; OP_INSTR[0xb4] = NES_OP_LDY; ADDR_MODE[0xb4] = NES_ADDR_zpx;
	OP_CYCLES[0xb5] = 4; OP_INSTR[0xb5] = NES_OP_LDA; ADDR_MODE[0xb5] = NES_ADDR_zpx;
	OP_CYCLES[0xb6] = 4; OP_INSTR[0xb6] = NES_OP_LDX; ADDR_MODE[0xb6] = NES_ADDR_zpy;
	OP_CYCLES[0xb7] = 2; OP_INSTR[0xb7] = NES_OP_NOP; ADDR_MODE[0xb7] = NES_ADDR_implied;
	OP_CYCLES[0xb8] = 2; OP_INSTR[0xb8] = NES_OP_CLV; ADDR_MODE[0xb8] = NES_ADDR_implied;
	OP_CYCLES[0xb9] = 4; OP_INSTR[0xb9] = NES_OP_LDA; ADDR_MODE[0xb9] = NES_ADDR_absy;
	OP_CYCLES[0xba] = 2; OP_INSTR[0xba] = NES_OP_TSX; ADDR_MODE[0xba] = NES_ADDR_implied;
	OP_CYCLES[0xbb] = 2; OP_INSTR[0xbb] = NES_OP_NOP; ADDR_MODE[0xbb] = NES_ADDR_implied;
	OP_CYCLES[0xbc] = 4; OP_INSTR[0xbc] = NES_OP_LDY; ADDR_MODE[0xbc] = NES_ADDR_absx;
	OP_CYCLES[0xbd] = 4; OP_INSTR[0xbd] = NES_OP_LDA; ADDR_MODE[0xbd] = NES_ADDR_absx;
	OP_CYCLES[0xbe] = 4; OP_INSTR[0xbe] = NES_OP_LDX; ADDR_MODE[0xbe] = NES_ADDR_absy;
	OP_CYCLES[0xbf] = 2; OP_INSTR[0xbf] = NES_OP_NOP; ADDR_MODE[0xbf] = NES_ADDR_implied;
	OP_CYCLES[0xc0] = 3; OP_INSTR[0xc0] = NES_OP_CPY; ADDR_MODE[0xc0] = NES_ADDR_immediate;
	OP_CYCLES[0xc1] = 6; OP_INSTR[0xc1] = NES_OP_CMP; ADDR_MODE[0xc1] = NES_ADDR_indx;
	OP_CYCLES[0xc2] = 2; OP_INSTR[0xc2] = NES_OP_NOP; ADDR_MODE[0xc2] = NES_ADDR_implied;
	OP_CYCLES[0xc3] = 2; OP_INSTR[0xc3] = NES_OP_NOP; ADDR_MODE[0xc3] = NES_ADDR_implied;
	OP_CYCLES[0xc4] = 3; OP_INSTR[0xc4] = NES_OP_CPY; ADDR_MODE[0xc4] = NES_ADDR_zp;
	OP_CYCLES[0xc5] = 3; OP_INSTR[0xc5] = NES_OP_CMP; ADDR_MODE[0xc5] = NES_ADDR_zp;
	OP_CYCLES[0xc6] = 5; OP_INSTR[0xc6] = NES_OP_DEC; ADDR_MODE[0xc6] = NES_ADDR_zp;
	OP_CYCLES[0xc7] = 2; OP_INSTR[0xc7] = NES_OP_NOP; ADDR_MODE[0xc7] = NES_ADDR_implied;
	OP_CYCLES[0xc8] = 2; OP_INSTR[0xc8] = NES_OP_INY; ADDR_MODE[0xc8] = NES_ADDR_implied;
	OP_CYCLES[0xc9] = 3; OP_INSTR[0xc9] = NES_OP_CMP; ADDR_MODE[0xc9] = NES_ADDR_immediate;
	OP_CYCLES[0xca] = 2; OP_INSTR[0xca] = NES_OP_DEX; ADDR_MODE[0xca] = NES_ADDR_implied;
	OP_CYCLES[0xcb] = 2; OP_INSTR[0xcb] = NES_OP_NOP; ADDR_MODE[0xcb] = NES_ADDR_implied;
	OP_CYCLES[0xcc] = 4; OP_INSTR[0xcc] = NES_OP_CPY; ADDR_MODE[0xcc] = NES_ADDR_absolute;
	OP_CYCLES[0xcd] = 4; OP_INSTR[0xcd] = NES_OP_CMP; ADDR_MODE[0xcd] = NES_ADDR_absolute;
	OP_CYCLES[0xce] = 6; OP_INSTR[0xce] = NES_OP_DEC; ADDR_MODE[0xce] = NES_ADDR_absolute;
	OP_CYCLES[0xcf] = 2; OP_INSTR[0xcf] = NES_OP_NOP; ADDR_MODE[0xcf] = NES_ADDR_implied;
	OP_CYCLES[0xd0] = 2; OP_INSTR[0xd0] = NES_OP_BNE; ADDR_MODE[0xd0] = NES_ADDR_relative;
	OP_CYCLES[0xd1] = 5; OP_INSTR[0xd1] = NES_OP_CMP; ADDR_MODE[0xd1] = NES_ADDR_indy;
	OP_CYCLES[0xd2] = 3; OP_INSTR[0xd2] = NES_OP_NOP; ADDR_MODE[0xd2] = NES_ADDR_implied;
	OP_CYCLES[0xd3] = 2; OP_INSTR[0xd3] = NES_OP_NOP; ADDR_MODE[0xd3] = NES_ADDR_implied;
	OP_CYCLES[0xd4] = 2; OP_INSTR[0xd4] = NES_OP_NOP; ADDR_MODE[0xd4] = NES_ADDR_implied;
	OP_CYCLES[0xd5] = 4; OP_INSTR[0xd5] = NES_OP_CMP; ADDR_MODE[0xd5] = NES_ADDR_zpx;
	OP_CYCLES[0xd6] = 6; OP_INSTR[0xd6] = NES_OP_DEC; ADDR_MODE[0xd6] = NES_ADDR_zpx;
	OP_CYCLES[0xd7] = 2; OP_INSTR[0xd7] = NES_OP_NOP; ADDR_MODE[0xd7] = NES_ADDR_implied;
	OP_CYCLES[0xd8] = 2; OP_INSTR[0xd8] = NES_OP_CLD; ADDR_MODE[0xd8] = NES_ADDR_implied;
	OP_CYCLES[0xd9] = 4; OP_INSTR[0xd9] = NES_OP_CMP; ADDR_MODE[0xd9] = NES_ADDR_absy;
	OP_CYCLES[0xda] = 3; OP_INSTR[0xda] = NES_OP_NOP; ADDR_MODE[0xda] = NES_ADDR_implied;
	OP_CYCLES[0xdb] = 2; OP_INSTR[0xdb] = NES_OP_NOP; ADDR_MODE[0xdb] = NES_ADDR_implied;
	OP_CYCLES[0xdc] = 2; OP_INSTR[0xdc] = NES_OP_NOP; ADDR_MODE[0xdc] = NES_ADDR_implied;
	OP_CYCLES[0xdd] = 4; OP_INSTR[0xdd] = NES_OP_CMP; ADDR_MODE[0xdd] = NES_ADDR_absx;
	OP_CYCLES[0xde] = 7; OP_INSTR[0xde] = NES_OP_DEC; ADDR_MODE[0xde] = NES_ADDR_absx;
	OP_CYCLES[0xdf] = 2; OP_INSTR[0xdf] = NES_OP_NOP; ADDR_MODE[0xdf] = NES_ADDR_implied;
	OP_CYCLES[0xe0] = 3; OP_INSTR[0xe0] = NES_OP_CPX; ADDR_MODE[0xe0] = NES_ADDR_immediate;
	OP_CYCLES[0xe1] = 6; OP_INSTR[0xe1] = NES_OP_SBC; ADDR_MODE[0xe1] = NES_ADDR_indx;
	OP_CYCLES[0xe2] = 2; OP_INSTR[0xe2] = NES_OP_NOP; ADDR_MODE[0xe2] = NES_ADDR_implied;
	OP_CYCLES[0xe3] = 2; OP_INSTR[0xe3] = NES_OP_NOP; ADDR_MODE[0xe3] = NES_ADDR_implied;
	OP_CYCLES[0xe4] = 3; OP_INSTR[0xe4] = NES_OP_CPX; ADDR_MODE[0xe4] = NES_ADDR_zp;
	OP_CYCLES[0xe5] = 3; OP_INSTR[0xe5] = NES_OP_SBC; ADDR_MODE[0xe5] = NES_ADDR_zp;
	OP_CYCLES[0xe6] = 5; OP_INSTR[0xe6] = NES_OP_INC; ADDR_MODE[0xe6] = NES_ADDR_zp;
	OP_CYCLES[0xe7] = 2; OP_INSTR[0xe7] = NES_OP_NOP; ADDR_MODE[0xe7] = NES_ADDR_implied;
	OP_CYCLES[0xe8] = 2; OP_INSTR[0xe8] = NES_OP_INX; ADDR_MODE[0xe8] = NES_ADDR_implied;
	OP_CYCLES[0xe9] = 3; OP_INSTR[0xe9] = NES_OP_SBC; ADDR_MODE[0xe9] = NES_ADDR_immediate;
	OP_CYCLES[0xea] = 2; OP_INSTR[0xea] = NES_OP_NOP; ADDR_MODE[0xea] = NES_ADDR_implied;
	OP_CYCLES[0xeb] = 2; OP_INSTR[0xeb] = NES_OP_NOP; ADDR_MODE[0xeb] = NES_ADDR_implied;
	OP_CYCLES[0xec] = 4; OP_INSTR[0xec] = NES_OP_CPX; ADDR_MODE[0xec] = NES_ADDR_absolute;
	OP_CYCLES[0xed] = 4; OP_INSTR[0xed] = NES_OP_SBC; ADDR_MODE[0xed] = NES_ADDR_absolute;
	OP_CYCLES[0xee] = 6; OP_INSTR[0xee] = NES_OP_INC; ADDR_MODE[0xee] = NES_ADDR_absolute;
	OP_CYCLES[0xef] = 2; OP_INSTR[0xef] = NES_OP_NOP; ADDR_MODE[0xef] = NES_ADDR_implied;
	OP_CYCLES[0xf0] = 2; OP_INSTR[0xf0] = NES_OP_BEQ; ADDR_MODE[0xf0] = NES_ADDR_relative;
	OP_CYCLES[0xf1] = 5; OP_INSTR[0xf1] = NES_OP_SBC; ADDR_MODE[0xf1] = NES_ADDR_indy;
	OP_CYCLES[0xf2] = 3; OP_INSTR[0xf2] = NES_OP_NOP; ADDR_MODE[0xf2] = NES_ADDR_implied;
	OP_CYCLES[0xf3] = 2; OP_INSTR[0xf3] = NES_OP_NOP; ADDR_MODE[0xf3] = NES_ADDR_implied;
	OP_CYCLES[0xf4] = 2; OP_INSTR[0xf4] = NES_OP_NOP; ADDR_MODE[0xf4] = NES_ADDR_implied;
	OP_CYCLES[0xf5] = 4; OP_INSTR[0xf5] = NES_OP_SBC; ADDR_MODE[0xf5] = NES_ADDR_zpx;
	OP_CYCLES[0xf6] = 6; OP_INSTR[0xf6] = NES_OP_INC; ADDR_MODE[0xf6] = NES_ADDR_zpx;
	OP_CYCLES[0xf7] = 2; OP_INSTR[0xf7] = NES_OP_NOP; ADDR_MODE[0xf7] = NES_ADDR_implied;
	OP_CYCLES[0xf8] = 2; OP_INSTR[0xf8] = NES_OP_SED; ADDR_MODE[0xf8] = NES_ADDR_implied;
	OP_CYCLES[0xf9] = 4; OP_INSTR[0xf9] = NES_OP_SBC; ADDR_MODE[0xf9] = NES_ADDR_absy;
	OP_CYCLES[0xfa] = 4; OP_INSTR[0xfa] = NES_OP_NOP; ADDR_MODE[0xfa] = NES_ADDR_implied;
	OP_CYCLES[0xfb] = 2; OP_INSTR[0xfb] = NES_OP_NOP; ADDR_MODE[0xfb] = NES_ADDR_implied;
	OP_CYCLES[0xfc] = 2; OP_INSTR[0xfc] = NES_OP_NOP; ADDR_MODE[0xfc] = NES_ADDR_implied;
	OP_CYCLES[0xfd] = 4; OP_INSTR[0xfd] = NES_OP_SBC; ADDR_MODE[0xfd] = NES_ADDR_absx;
	OP_CYCLES[0xfe] = 7; OP_INSTR[0xfe] = NES_OP_INC; ADDR_MODE[0xfe] = NES_ADDR_absx;
	OP_CYCLES[0xff] = 2; OP_INSTR[0xff] = NES_OP_NOP; ADDR_MODE[0xff] = NES_ADDR_implied;
}
void NES_exec()
{
	OPERAND_VAL = 0;
	OPERAND_ADDR = 0;
	OPCODE = memory[PC++];
	clockTicks += OP_CYCLES[OPCODE];
	ADDR_MODE[OPCODE]();
	byte addr = ((OPCODE & 0xF0) >> 4) * 0x10 + (OPCODE & 0x0F);
	printf("%s(0x%X)\n  BEFORE: OA:$0x%X-OV:0x%X | A=0x%X;X=0x%X;Y=0x%X;S=%s;SP=0x%X;PC=0x%X\n",
		OP_NAME[addr].c_str(), (int)OPCODE, &OPERAND_ADDR, *OPERAND_ADDR, A, X, Y, S.to_string().c_str(), SP, PC);
	OP_INSTR[OPCODE]();
	printf("   AFTER: OA:$0x%X-OV:0x%X | A=0x%X;X=0x%X;Y=0x%X;S=%s;SP=0x%X;PC=0x%X\n",
		&OPERAND_ADDR, *OPERAND_ADDR, A, X, Y, S.to_string().c_str(), SP, PC);
}
void NES_doCPUTicks()
{

}

/* DEBUGGING */
void NES_setInstructions(byte* inst, int len)
{
	for (int i = 0; i < len; i++)
	{
		memory[0x0600 + i] = inst[i];
	}
}

