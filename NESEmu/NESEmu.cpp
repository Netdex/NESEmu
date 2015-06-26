// NESEmu.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <bitset>
#include <iostream>
using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
	char test[10];
	fill_n(test, 10, 'A');
	test[9] = '\0';
	char* tester = test + 5;
	tester[3] = '1';
	cout << test;
	cin.get();
	return 0;
}

