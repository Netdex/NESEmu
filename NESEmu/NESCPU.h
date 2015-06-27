#pragma once
#include <bitset>
#include "StatusFlag.h"
#include <iostream>

typedef unsigned char byte;
typedef unsigned short word;

void NES_init();
void NES_exec();
void NES_doCPUTicks(int n);
void NES_setInstructions(byte* inst, int len);

