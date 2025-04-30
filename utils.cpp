#include "utils.h"
#include "common.h"
#include <iostream>
#include <stdexcept>
#include <vector>


// getBit 
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