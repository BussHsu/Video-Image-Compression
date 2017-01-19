#include <iostream>
#include <fstream>
#include "LZW.h"


void testEncode()
{
	unsigned char* testInput = new unsigned char[16]{ 39,39,126,126,39,39,126,126,39,39,126,126,39,39,126,126 };
	LZW lzw;
	lzw.encode(testInput, 16, "testEncode.bin");
	delete[] testInput;
}

void testDecode()
{
	LZWDecoder decoder;
	decoder.decode("testEncode.bin", "testDecode.txt");
}

int main() 
{
	testDecode();
	int a;
	std::cin >> a;
	return 0;
}