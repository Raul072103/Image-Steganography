#include "utils.h"
#include "secret.h"
#include "common.h"
#include <iostream>
#include <stdexcept>
#include <vector>


void decodeMessage() {
	// get image
	// choose decoding method
	int encodingMethod;

	// 3 options - LSB, PVD, BSP
	printf(" 0 - Exit");
	printf(" 1 - LSB\n");
	scanf("%d", &encodingMethod);
	printf("\n");

	switch (encodingMethod)
	{
	case 0:
		break;

		// LSB
	case 1:
		break;

	default:
		printf("You can only choose encode or decode\n");
		break;
	}

}

bool getBit(std::vector<byte>& secret, int n) {

	// 1 byte = 8 bits => total = secretLength * 8
	if (n < 0 || n >= secret.size() * 8) {
		throw std::out_of_range("getBit: bit index out of range");
	}
	
	unsigned int bytePostion = n / 8;
	unsigned int bitPosition = n % 8;

	byte selectedByte = secret[bytePostion];

	byte bitMask = 0b00000001 << bitPosition;

	// shift selected bit on first position so we can detect if it is a 0 or 1
	return ((selectedByte & bitMask) >> bitPosition) == 1;
}
