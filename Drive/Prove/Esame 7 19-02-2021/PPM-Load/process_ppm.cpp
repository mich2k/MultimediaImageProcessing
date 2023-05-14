#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdbool>
#include <cctype>
#include <sstream>
#include "mat.h"
#include "ppm.h"

void ignoreEventualComments(std::istream& is){
	if (is.peek() == '#') {
		is.get();
		std::string comment;
		char terminator = 0x0A;
		std::getline(is, comment, terminator);
	}
}

uint16_t readTwoBytesInBigEndian(std::istream& is) {
	uint8_t mostSign = is.get();
	uint8_t lessSign = is.get();
	std::stringstream swappedStream;
	swappedStream.write(reinterpret_cast<char*>(&lessSign), 1);
	swappedStream.write(reinterpret_cast<char*>(&mostSign), 1);
	uint16_t beNumber;
	swappedStream.read(reinterpret_cast<char*>(&beNumber), 2);
	return beNumber;
}

bool headerReader(std::istream& is, unsigned& w, unsigned& h, bool& oneBytePerPixel) {

	std::string lineRead;
	is >> lineRead;
	if (lineRead != "P6") { return false; } // Magic number
	if (!isspace(is.get())) { return false; } // Whitespace
	ignoreEventualComments(is);
	is >> lineRead;
	w = stoi(lineRead); // w
	if (!isspace(is.get())) { return false; } // Whitespace
	ignoreEventualComments(is);
	is >> lineRead;
	h = stoi(lineRead); // h
	if (!isspace(is.get())) { return false; } // Whitespace
	ignoreEventualComments(is);
	is >> lineRead;
	int maxVal = stoi(lineRead); // maxVal
	if (maxVal <= 0 || maxVal >= 65536) { return false; }
	if (maxVal >= 256)
		oneBytePerPixel = false;
	if (!isspace(is.get())) { return false; } // Whitespace
	ignoreEventualComments(is);

	return true;
}

bool LoadPPM(const std::string& filename, mat<vec3b>& img) {

	// Apro il file di input:
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	// Leggo l'header
	unsigned w, h;
	bool oneBytePerPixel = true; // True: 1 Byte, False: 2 Byte
	if (!headerReader(is, w, h, oneBytePerPixel)) {
		return false;
	}

	// Inizializzo la matrice:
	if (oneBytePerPixel) {
		img.resize(h, w);
		for (size_t r = 0; r < h; ++r) {
			for (size_t c = 0; c < w; ++c) {
				vec3b tmp;
				is.read(reinterpret_cast<char*>(&tmp[0]), 1);
				is.read(reinterpret_cast<char*>(&tmp[1]), 1);
				is.read(reinterpret_cast<char*>(&tmp[2]), 1);
				img(r, c) = tmp;
			}
		}
	}
	else {
		img.resize(h, 2 * w);
		for (size_t r = 0; r < h; ++r) {
			for (size_t c = 0; c < w; ++c) {
				vec3b tmp;
				tmp[0] = readTwoBytesInBigEndian(is);
				tmp[1] = readTwoBytesInBigEndian(is);
				tmp[2] = readTwoBytesInBigEndian(is);
				img(r, c) = tmp;
			}
		}
	}

	return true;
}
