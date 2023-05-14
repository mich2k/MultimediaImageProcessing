#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "mat.h"
#include "types.h"
#include "utils.h"

struct rate {
	size_t num, den;
};

struct streamHeader {

	size_t W = 0, H = 0;
	std::string C = "420jpeg";
	char I = 'p';
	rate F, A;
	std::string X;
};

struct y4m {

	streamHeader header;
	std::vector<mat<uint8_t>> frames;

	bool readHeader(std::istream& is) {
		std::string field;
		is >> field;
		if (field != "YUV4MPEG2") { return false; }
		if (is.get() != ' ') { return false; }
		while (true) {
			char letterCode = is.get();
			is >> field;
			if (letterCode == 'W') { header.W = stoi(field); }
			if (letterCode == 'H') { header.H = stoi(field); }
			if (is.peek() == 0x0A) break;
			if (is.get() != ' ') return false;
		}
		std::getline(is, field, (char)0x0A);
		return true;
	}

	bool readFrames(std::istream& is) {
		std::string frameHeader;
		mat<uint8_t> Y(header.H, header.W);
		mat<uint8_t> Cb(header.H / 2, header.W / 2);
		mat<uint8_t> Cr(header.H / 2, header.W / 2);
		while (std::getline(is, frameHeader, (char)0x0A)) {
			if (frameHeader.find("FRAME") == std::string::npos) break;
			is.read(Y.rawdata(), Y.rawsize());
			is.read(Cb.rawdata(), Cb.rawsize());
			is.read(Cr.rawdata(), Cr.rawsize());
			frames.push_back(Y);
		}
		return true;
	}
};

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	// Apro il file di input
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "\n\tImpossibile aprire il file di input " + filename + "\n";
		return false;
	}
	// Leggo il file y4m
	y4m img;
	if (!img.readHeader(is)) {
		std::cout << "\n\tFile " + filename + " non formattato correttamente.\n";
		return false;
	}
	if (!img.readFrames(is)) {
		std::cout << "\n\tFile " + filename + " non formattato correttamente.\n";
		return false;
	}
	frames = img.frames;
	return true;
}
