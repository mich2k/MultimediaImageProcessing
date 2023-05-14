#include "pbm.h"

struct bitreader {

	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {}

	int read_bit() {
		if (nbits_ == 0) {
			if (!is_.read(reinterpret_cast<char*>(&buffer_), 1)) {
				return EOF;
			}
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	explicit operator bool() { return bool(is_); }
	bool operator!() { return !is_; }

	std::istream& read(uint8_t& u, uint8_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

void skipComment(std::istream& is) {
	if (is.peek() == '#') {
		std::string commentToFlush;
		std::getline(is, commentToFlush, '\n');
		is.get();
	}
}

bool BinaryImage::readHeader(std::istream& is) {
	std::string stringRead;
	is >> stringRead;
	if (stringRead != "P4") return false; // Magic number
	if (is.get() != 0x0A) return false; // '\n'
	skipComment(is);
	is >> stringRead; // W
	W = stoi(stringRead);
	if (is.get() != ' ') return false; // ' '
	is >> stringRead; // H
	H = stoi(stringRead);
	if (is.get() != '\n') return false; // '\n'
	return true;
}

bool BinaryImage::readDataRaster(std::istream& is){
	bitreader br(is);
	uint8_t dontCareBits = W % 8;
	for (int r = 0; r < H; ++r) {
		for (int c = 0; c < W / 8; ++c) {
			uint8_t pixel;
			br.read(pixel, 8);
			ImageData.push_back(pixel);
		}
		if (dontCareBits != 0) {
			uint8_t pixel;
			br.read(pixel, 8);
			ImageData.push_back(pixel);
		}
	}
	return true;
}


bool BinaryImage::ReadFromPBM(const std::string& filename) {
	// Apertura del file:
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "\n\tERRORE: Impossibile aprire il file di input " + filename + "\n";
		return false;
	}
	if (!readHeader(is)) {
		std::cout << "\n\tERRORE: Il file " + filename + " non è nel formato pbm corretto.\n";
		return false;
	}
	if (!readDataRaster(is)) {
		std::cout << "\n\tERRORE: Lettura dei dati dell'immagine non riuscita.\n";
		return false;
	}
	return true;
}
