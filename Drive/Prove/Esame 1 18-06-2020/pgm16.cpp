#include "pgm16.h"

struct headerPgm {
	std::string magicNumber;
	size_t width;
	size_t height;
	uint16_t maxvalue;
};

struct pgm {

	headerPgm header;
	mat<uint16_t> data;

	bool readHeader(std::istream& is) {
		skipComment(is);
		is >> header.magicNumber; // Magic number
		if (header.magicNumber != "P5") return false;
		if (!isspace(is.get())) return false; // Whitespace
		std::string tmp;
		skipComment(is);
		is >> tmp; // Width
		header.width = stoi(tmp);
		if (!isspace(is.get())) return false; // Whitespace
		skipComment(is);
		is >> tmp; // Height
		header.height = stoi(tmp);
		if (!isspace(is.get())) return false; // Whitespace
		skipComment(is);
		is >> tmp; // Maxvalue
		header.maxvalue = stoi(tmp);
		if (!isspace(is.get())) return false; // Whitespace
		skipComment(is);

		return true;
	}

	bool readDataRaster(std::istream& is) {
		data.resize(header.height, header.width); // Predispozione della matrice
		for (int r = 0; r < data.rows(); ++r) {
			for (int c = 0; c < data.cols(); ++c) {
				if (header.maxvalue < 256) { // 8 bit per pixel
					if (!is.read(reinterpret_cast<char*>(&data(r, c)), 1)) { return false; }
				}
				else {
					uint16_t LEval;
					if (!is.read(reinterpret_cast<char*>(&LEval), 2)) { return false; }
					data(r, c) = LEtoBE(LEval);
				}
			}
		}
		return true;
	}

	// Utilities
private:
	void skipComment(std::istream& is) {
		if (is.peek() == '#') {
			std::string tmp;
			while (is.get() != 0x0A) { 
				// flush comment
			}
		}
	}

	uint16_t LEtoBE(uint16_t& LEval) {
		uint16_t BEval = 0;
		BEval = LEval << 8;
		BEval += LEval >> 8;
		return BEval;
	}
};

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {

	// Apro il file ricevuto in input:
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "\n\tERRORE: Impossibile aprire il file di input " + filename + "\n";
		return false;
	}

	pgm inputImage;
	if (!inputImage.readHeader(is)) {
		std::cout << "\n\tERRORE: Header non formattato correttamente.\n";
		return false;
	}
	if (!inputImage.readDataRaster(is)) {
		std::cout << "\n\tERRORE: Problema durante la lettura dei dati.\n";
		return false;
	}

	img = inputImage.data;
	maxvalue = inputImage.header.maxvalue;

	return true;
}
