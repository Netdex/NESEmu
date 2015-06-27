#pragma once
#include <bitset>
#include "StatusFlag.h"

typedef unsigned char byte;
typedef unsigned short word;

void init();

void exec();
void doCPUTicks(int n);

void implied();		// NO OPERAND ADDRESS
void immediate();	// [OP VALUE] given by inst (LDA #$0A)
void abs();			// [OP ADDR] given by inst (LDA $31F6)
void relative();		// [REL ADDR VALUE] given by inst, then added as signed byte onto PC (BEQ $A7)
void indirect();		// [OP ADDR] given by inst, next byte added to make JMP address (JMP ($215F))
void absx();			// [REL OP ADDR] given by inst, then added to X reg to get op address (STA $3000,X)
void absy();			
void zp();			// [OP ADDR] given by inst, 1 byte
void zpx();			// [OP ADDR] given by inst, add to X reg, wrap around 0xff!
void zpy();
void indx();			// [OP ADDR] taken from inst, addr is added to X reg with wrap
void indy();
void accum();		// [OP ADDR] is the accumulator

void ADC();		// Add Memory to Accumulator with Carry
void AND();		// "AND" Memory with Accumulator
void ASL();	// Shift Left One Bit (Memory or Accumulator)

void BCC();		// Branch on Carry Clear
void BCS();		// Branch on Carry Set
void BEQ();		// Branch on Result Zero
void BIT();		// Test Bits in Memory with Accumulator
void BMI();		// Branch on Result Minus
void BNE();		// Branch on Result not Zero
void BPL();		// Branch on Result Plus
void BRK();				// Force Break
void BVC();		// Branch on Overflow Clear
void BVS();		// Branch on Overflow Set

void CLC();				// Clear Carry Flag
void CLD();				// Clear Decimal Mode
void CLI();				// Clear interrupt Disable Bit
void CLV();				// Clear Overflow Flag
void CMP();		// Compare Memory and Accumulator
void CPX();		// Compare Memory and Index X
void CPY();		// Compare Memory and Index Y

void DEC();	// Decrement Memory by One
void DEX();				// Decrement Index X by One
void DEY();				// Decrement Index Y by One
void EOR();		// "Exclusive-Or" Memory with Accumulator

void INC();	// Increment Memory by One
void INX();				// Increment Index X by One
void INY();				// Increment Index Y by One

void JMP();		// Jump to New Location
void JSR();		// Jump to New Location Saving Return Address

void LDA();		// Load Accumulator with Memory
void LDX();		// Load Index X with Memory
void LDY();		// Load Index Y with Memory 
void LSR();	// Shift Right One Bit(Memory or Accumulator)

void NOP();				// No Operation

void ORA();		// "OR" Memory with Accumulator

void PHA();				// Push Accumulator on Stack
void PHP();				// Push Processor Status on Stack
void PLA();				// Pull Accumulator from Stack
void PLP();				// Pull Processor Status from Stack

void ROL();	// Rotate One Bit Left(Memory or Accumulator)
void ROR();	// Rotate One Bit Right(Memory or Accumulator)
void RTI();				// Return from Interrupt
void RTS();				// Return from Subroutine

void SBC();		// Subtract Memory from Accumulator with Borrow
void SEC();				// Set Carry Flag
void SED();				// Set Decimal Mode
void SEI();				// Set Interrupt Disable Status
void STA();	// Store Accumulator in Memory
void STX();	// Store Index X in Memory
void STY();	// Store Index Y in Memory

void TAX();				// Transfer Accumulator to Index X
void TAY();				// Transfer Accumulator to Index Y
void TSX();				// Transfer Stack Pointer to Index X
void TXA();				// Transfer Index X to Accumulator
void TXS();				// Transfer Index X to Stack Pointer
void TYA();				// Transfer Index Y to Accumulator


