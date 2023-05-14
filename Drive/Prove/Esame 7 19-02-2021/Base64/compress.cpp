#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdbool>
#include <cctype>
#include <sstream>
#include <iterator>
#include "mat.h"
#include "ppm.h"

char base64Converter(uint8_t val) {
	if (val <= 25) { return val + 65; }
	if (val >= 26 && val <= 51) { return val + 71; }
	if (val >= 52 && val <= 61) { return val - 4; }
	if (val == 62) { return '+'; }
	if (val == 63) { return '/'; }
	return '='; // ERROR CODE
}

struct bitreader {

	uint8_t buffer_ = 0; // Anche se non è necessario, ma il compilatore di VS 2019 altrimenti si lamenta.
	uint8_t nbits_ = 0;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {}

	int read_bit() {
		if (nbits_ == 0) {
			if(!is_.read(reinterpret_cast<char*>(&buffer_), 1))
				return EOF;
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

void fillStream(const std::vector<uint8_t>& v, std::ostream& os) {
	for (auto& carattere : v) {
		uint8_t elem = carattere;
		os.write(reinterpret_cast<char*>(&elem), 1);
	}
}

std::string Base64Encode(const std::vector<uint8_t>& v) {

	// Converto il vettore in uno stringstream
	std::string input;
	std::stringstream stream(input);
	fillStream(v, stream);

	// Stream stuffing (se necessario)
	size_t inputSize = v.size();
	uint8_t stuff = 128;
	while (inputSize % 3 != 0) {
		stream.write(reinterpret_cast<char*>(&stuff), 1);
		++inputSize;
	}

	// Con un bitreader leggo il contenuto dello stream 6 bit alla volta e lo memorizzo in un vettore:
	std::vector<uint8_t> sixBitsOutput;
	bitreader br(stream);
	uint8_t readVal;
	while (br.read(readVal, 6)) {
		sixBitsOutput.push_back(readVal);
	}

	// Riporto i valori convertiti sulla stringa di output:
	std::string output;
	for (auto& carattere : sixBitsOutput) {
		output.push_back(base64Converter(carattere));
	}

	return output;
}
