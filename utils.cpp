#include "utils.h"
#include "common.h"
#include <iostream>

bool getBit(byte* secret, int secretLength, int n) {

	// 1 byte = 8 bits => total = secretLength * 8

	unsigned int totalBits = secretLength * 8;
	
	unsigned int bytePostion = totalBits / n;
	unsigned int bitPosition = totalBits % n;

	byte selectedByte = secret[bytePostion];

	byte bitMask = 0b00000001 << bitPosition;

	return secret[selectedByte] & bitMask;
}