#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <bitset>

void syntax() {
	std::cout << "\n\tSintassi del programma snappy:"
		<< "\n\tsnappy <input_file> <output_file>\n\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& errorMessage) {
	std::cout << "\n\tERRORE: " + errorMessage + "\n\n";
	exit(EXIT_FAILURE);
}

struct bitreader {

	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {};

	int read_bit() {
		if (nbits_ == 0) {
			if (!is_.read(reinterpret_cast<char*>(&buffer_), 1))
				return EOF;
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	explicit operator bool() { return bool(is_); }
	bool operator!() { return !(is_); }

	std::istream& read(uint8_t& u, uint8_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	int read_bit_mod(std::stringstream& stream) {
		if (nbits_ == 0) {
			if (!stream.read(reinterpret_cast<char*>(&buffer_), 1))
				return EOF;
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	std::istream& readVarint(size_t& u) {
		u = 0;
		std::stringstream streamLE;
		std::vector<uint8_t> varints;
		uint8_t value;
		while (true) {
			is_.read(reinterpret_cast<char*>(&value), 1);
			varints.push_back(value);
			if ((value >> 7) == 0) // Se l'indicatore è 0, mi fermo
				break;
		}
		for (int i = varints.size() - 1; i >= 0; --i) {
			streamLE.write(reinterpret_cast<char*>(&varints[i]), 1);
		}
		for (size_t i = 0; i < varints.size(); ++i) {
			uint8_t indicator = read_bit_mod(streamLE);
			for (uint8_t i = 0; i < 7; ++i) // Leggo i sette bit
				u = (u << 1) | read_bit_mod(streamLE); // e ne riporto il valore su u
		}
		return is_;
	}
};

struct bitwriter {

	uint8_t buffer_ = 0;
	uint8_t nbits_ = 0;
	std::stringstream& os_;

	bitwriter(std::stringstream& os) : os_(os) {}
	~bitwriter() { flush(); }

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

	std::stringstream& write(uint8_t u, uint8_t n) {
		while (n-- > 0) {
			uint8_t bit = (u >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	std::stringstream& operator() (uint8_t u, uint8_t n) {
		return write(u, n);
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}
};

struct SnappyDecoder{

	std::istream& is_;
	std::ostream& os_;
	std::vector<uint8_t> decompressedStream;

	SnappyDecoder(std::istream& is, std::ostream& os) : is_(is), os_(os) {};

	size_t readPreamble() { 
		size_t originalLength;
		bitreader br(is_);
		br.readVarint(originalLength);
		return originalLength;
	}

	void decompress() {
		uint8_t valRead;
		// Leggo il flusso compresso
		while (is_.read(reinterpret_cast<char*>(&valRead), 1)) {
			if (valRead == EOF)
				break;
			uint8_t tag = valRead & 0b00000011; // Due bit meno significativi
			uint8_t value = (valRead & 0b11111100) >> 2; // Sei bit più significativi
			if (tag == 0b00) { // Literal
				readLiteral(value);
			}
			if (tag == 0b01) { // Copy with 1-byte offset
				copyWith1ByteOffset(valRead);
			}
			if (tag == 0b10) { // Copy with 2-byte offset
				copyWith2ByteOffset(valRead);
			}
			if (tag == 0b11) { // Copy with 4-byte offset
				copyWith4ByteOffset(valRead);
			}
		}
		writeDecompressedOutput();
	}

	void readLiteral(uint8_t value) {
		size_t length = 0;
		std::vector<uint8_t> literal;
		// Lengths
		if (value < 60) { // Literals fino a 60 bytes. Attenzione! Ho messo < 60 per via del + 1
			length = value;
		}
		if (value == 60) { is_.read(reinterpret_cast<char*>(&length), 1); }
		if (value == 61) { is_.read(reinterpret_cast<char*>(&length), 2); }
		if (value == 62) { is_.read(reinterpret_cast<char*>(&length), 3); }
		if (value == 63) { is_.read(reinterpret_cast<char*>(&length), 4); }
		length = length + 1; // Per via del + 1
		// Literal
		literal.resize(length);
		is_.read(reinterpret_cast<char*>(literal.data()), length); // Leggo il literal
		decompressedStream.insert(decompressedStream.end(), literal.begin(), literal.end()); // Riporto il literal nello stream decompresso
	}

	void copyWith1ByteOffset(uint8_t value) {
		uint8_t length = ((value & 0b00011100) >> 2)+ 4; // Length
		uint16_t offset = (value & 0b11100000) << 3;
		uint8_t nextByte;
		is_.read(reinterpret_cast<char*>(&nextByte), 1);
		offset = offset + nextByte; // I bit di nextByte andranno ad occupare gli otto bit meno significativi dell'offset
		std::vector<uint8_t> copy = getCopy(length, offset); // Leggo il contenuto della copy
		decompressedStream.insert(decompressedStream.end(), copy.begin(), copy.end()); // Aggiungo la copy allo stream decompresso
	}

	void copyWith2ByteOffset(uint8_t value) {
		uint8_t length = ((value & 0b11111100) >> 2) + 1; // Length
		uint16_t offset;
		is_.read(reinterpret_cast<char*>(&offset), 2); // Leggo l'offset memorizzato nei due Byte successivi in LE
		std::vector<uint8_t> copy = getCopy(length, offset);
		decompressedStream.insert(decompressedStream.end(), copy.begin(), copy.end()); // Aggiungo la copy allo stream decompresso
	}

	void copyWith4ByteOffset(uint8_t value) {
		uint8_t length = ((value & 0b11111100) >> 2) + 1; // Length
		uint32_t offset;
		is_.read(reinterpret_cast<char*>(&offset), 4); // Leggo l'offset memorizzato nei quattro Byte successivi in LE
		std::vector<uint8_t> copy = getCopy(length, offset);
		decompressedStream.insert(decompressedStream.end(), copy.begin(), copy.end()); // Aggiungo la copy allo stream decompresso
	}

	void writeDecompressedOutput() {
		for (auto& character : decompressedStream)
			os_.write(reinterpret_cast<char*>(&character), 1);
	}

	std::vector<uint8_t> getCopy(size_t length, size_t offset) {
		std::vector<uint8_t> copyContent;
		size_t chRead = 0;
		while (chRead < length) { // Finché non ho letto length caratteri
			for (size_t i = decompressedStream.size() - offset; i < decompressedStream.size(); ++i) { // Riparto dalla posizione indicata dall'offset
				copyContent.push_back(decompressedStream[i]); // Leggo un carattere
				++chRead; // e incremento il contatore
				if (chRead == length) { break; } // se ho letto length caratteri, esco
			}
		}
		return copyContent;
	}
};

int main(int argc, char** argv) {

	// Linea di comando
	if (argc != 3) {
		syntax();
	}

	std::string inputFilename = argv[1];
	std::string outputFilename = argv[2];

	std::ifstream is(inputFilename, std::ios::binary);
	if (!is) {
		error("Impossibile aprire il file di input " + inputFilename);
	}
	std::ofstream os(outputFilename, std::ios::binary);
	if (!os) {
		error("Impossible aprire il file di output " + outputFilename);
	}

	SnappyDecoder decoder(is, os);
	size_t originalLength = decoder.readPreamble();
	decoder.decompress();

	return EXIT_SUCCESS;
}
