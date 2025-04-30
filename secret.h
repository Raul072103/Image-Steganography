#pragma once

#include "common.h"
#include <string>
#include <variant>

/*
	
	Secrets are going to be encoded into pictures.

	Each image steganography will have 2 types of main methods:
		
		Mat encode(std::vector<byte> secret);
		std::vector<byte> decode(Mat encodedMsg);

	Each secret will me encoded inside an image chosen by the user.
	Each encoded image will be decoded into an array of bytes.

	To handle this 2 standard methods a way to include metadata into
	secret also must be introduced for easier portability of different
	secret formats.

	The secret formats accepted will be: files, images, strings.

	The secret will be encoded in the following format:

	secret_bytes = xxxxxx..xxxx => Secret header
				 = xxxxxx..xxxx =>
				 = xxxxxx..xxxx 	=>
				 = xxxxxx..xxxx 		=> Secret message (size = header.secretSize)
				 = ....				=> 
				 = xxxxxx..xxxx => 

*/

enum SecretFormat {
	FILE,
	IMAGE,
	STRING
};

struct SecretHeader {
	SecretFormat format;	 // format of the secret encoded
	std::string name;		 // name contains also the extension (.png, .jpg, .txt, .exe etc)
	unsigned int secretSize; // secret size in bits
};

std::vector<byte> encodeSecretHeader(SecretHeader header);
SecretHeader decodeSecretHeader(std::vector<byte>& secret);
