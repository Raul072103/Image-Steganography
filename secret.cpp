#include "secret.h"
#include "common.h"

#include <iostream>
#include <vector>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>  // for memcpy

// Helper: write 4-byte unsigned int in big-endian
void writeUInt(std::vector<byte>& buffer, unsigned int value) {
	for (int i = 3; i >= 0; --i) {
		buffer.push_back((value >> (i * 8)) & 0xFF);
	}
}

// Helper: read 4-byte unsigned int from offset
unsigned int readUInt(const std::vector<byte>& buffer, size_t& offset) {
	if (offset + 4 > buffer.size())
		throw std::out_of_range("readUInt: buffer overrun");

	unsigned int value = 0;
	for (int i = 0; i < 4; ++i) {
		value = (value << 8) | buffer[offset++];
	}
	return value;
}

std::vector<byte> encodeSecretHeader(SecretHeader header) {
	std::vector<byte> result;

	// Write placeholder for header size (we'll fix this at the end)
	writeUInt(result, 0); // placeholder

	// Write format
	writeUInt(result, static_cast<unsigned int>(header.format));

	// Write name
	writeUInt(result, header.name.size());
	result.insert(result.end(), header.name.begin(), header.name.end());

	// Write secretSize
	writeUInt(result, header.secretSizeBits);

	// Now go back and write the correct header size (total length)
	unsigned int actualSize = result.size();
	result[0] = (actualSize >> 24) & 0xFF;
	result[1] = (actualSize >> 16) & 0xFF;
	result[2] = (actualSize >> 8) & 0xFF;
	result[3] = actualSize & 0xFF;

	return result;
}

SecretHeader decodeSecretHeader(std::vector<byte>& buffer) {
	size_t offset = 0;

	unsigned int headerSize = readUInt(buffer, offset);
	unsigned int formatInt = readUInt(buffer, offset);
	if (formatInt > 2)
		throw std::invalid_argument("Invalid SecretFormat");

	unsigned int nameSize = readUInt(buffer, offset);
	if (offset + nameSize > buffer.size())
		throw std::out_of_range("decodeSecretHeader: name exceeds buffer");

	std::string name(reinterpret_cast<const char*>(&buffer[offset]), nameSize);
	offset += nameSize;

	unsigned int secretSize = readUInt(buffer, offset);

	return SecretHeader{
		.headerSizeBytes = headerSizeBytes,
		.format = static_cast<SecretFormat>(formatInt),
		.name = name,
		.secretSizeBits = secretSizeBits
	};
}
