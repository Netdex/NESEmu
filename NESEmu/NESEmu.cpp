// NESEmu.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <bitset>
#include <iostream>
#include "NESCPU.h"

using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
	NES_init();
	byte inst[] = {0xa9,0x01,0x85,0xf0,0xa9,0xcc,0x85,0xf1,0x6c,0xf0,0x00 };
	NES_setInstructions(inst, sizeof(inst) / sizeof(byte));
	while (true)
	{
		cin.get();
		NES_exec();
	}
	return 0;
}

