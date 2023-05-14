#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void syntax() {
	std::cout << "\n\tSintassi del programma lz4decomp:"
		<< "\n\tlz4decomp <input_filename> <output_filename>\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& message) {
	std::cout << "\n\tERRORE: " + message + "\n";
	exit(EXIT_FAILURE);
}

class lz4_decoded_sequence {

	std::vector<uint8_t> sequence_;

public:
	void insert_sequence(std::vector<uint8_t>& sequence) {
		for (auto& byte : sequence) {
			sequence_.push_back(byte);
		}
	}
	std::vector<uint8_t> get_sequence(size_t offset, size_t match_length) {
		std::vector<uint8_t> sequence;
		while (offset > sequence_.size())
			offset -= sequence_.size();
		int starting_point = sequence_.size() - offset;
		while (match_length > offset) {
			for (int index = starting_point; index < sequence_.size(); ++index) {
				sequence.push_back(sequence_[index]);
			}
			match_length -= offset;
		}
		for (int index = starting_point; index < starting_point + match_length; ++index) {
			sequence.push_back(sequence_[index]);
		}
		return sequence;
	}
};

template <typename T>
void output_data(std::ostream& os, std::vector<T> data) {
	for (auto& element : data) {
		os.write(reinterpret_cast<char*>(&element), sizeof(element));
	}
}

void read_lz4_header(std::istream& is, uint32_t& uncompressed_file_length) {
	/*
	std::vector<uint8_t> magicNumber;
	for (uint8_t i = 0; i < 4; ++i) {
		uint8_t code;
		is.read(reinterpret_cast<char*>(&code), 1);
		magicNumber.push_back(code);
	}
	// 03 21 4C 18 --> Magic number corretto
	if (magicNumber[0] != 0x03 || magicNumber[1] != 0x21 || magicNumber[2] != 0x4C || magicNumber[3] != 18) {
		error("Il file di input non è formattato correttamente.");
	}*/
	// Controllo il magic number
	uint32_t magicNumber;
	is.read(reinterpret_cast<char*>(&magicNumber), 4);
	if (magicNumber != 0x184C2103) {
		error("Il file di input non è formattato correttamente.");
	}
	// Lunghezza del file non compresso
	is.read(reinterpret_cast<char*>(&uncompressed_file_length), 4);
	// Leggo il valore costante
	uint32_t constantValue;
	is.read(reinterpret_cast<char*>(&constantValue), 4);
	if (constantValue != 0x4D000000) {
		error("Il file di input non è formattato correttamente.");
	}
}

void lz4decomp(std::istream& is, std::ostream& os) {

	uint32_t uncompressed_file_length;
	read_lz4_header(is, uncompressed_file_length);
	uint8_t token;
	lz4_decoded_sequence dictionary;
	uint32_t blockLength;
	while (is.read(reinterpret_cast<char*>(&blockLength), 4)) {
		uint32_t byteReadInBlock = 0;
		while (is.read(reinterpret_cast<char*>(&token), 1)) {
			++byteReadInBlock;
			size_t literal_length = (token & 0b11110000) >> 4;
			size_t match_length = token & 0b00001111;
			// if (literal_length == 0) { } no literal
			if (literal_length == 15) { // Ho bisogno di leggere altri byte per calcolare la lunghezza totale:
				uint8_t byte;
				while (is.read(reinterpret_cast<char*>(&byte), 1)) {
					literal_length += byte;
					++byteReadInBlock;
					if (byte != 255) { break; } // Proseguo solo se ho 255
				}
			}
			// Leggo un numero di letterali pari a literal_length
			std::vector<uint8_t> literals;
			for (size_t i = 0; i < literal_length; ++i) {
				uint8_t literal;
				is.read(reinterpret_cast<char*>(&literal), 1);
				literals.push_back(literal);
				++byteReadInBlock;
			}
			output_data(os, literals); // li riporto in output
			dictionary.insert_sequence(literals); // e all'interno della sequenza decodificata
			if (byteReadInBlock == blockLength) // So che un blocco termina con i letterali: controllo il contatore qui
				break;
			// Leggo l'offset
			uint16_t offset;
			is.read(reinterpret_cast<char*>(&offset), 2);
			byteReadInBlock += 2;
			// Leggo la lunghezza del match
			if (match_length == 15) { // Ho bisogno di leggere altri byte per calcolare la lunghezza totale:
				uint8_t byte;
				while (is.read(reinterpret_cast<char*>(&byte), 1)) {
					match_length += byte;
					++byteReadInBlock;
					if (byte != 255) { break; } // Proseguo solo se ho 255
				}
			}
			match_length += 4; // Incremento il valore di minmatch
			std::vector<uint8_t> run = dictionary.get_sequence(offset, match_length);
			dictionary.insert_sequence(run);
			output_data(os, run); // Riporto il match sull'output
		}
	}
}

int main(int argc, char** argv) {

	// Command line management
	if (argc != 3) {
		syntax();
	}
	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	std::ifstream is(input_filename, std::ios::binary);
	if (!is) { error("Impossibile aprire il file di input " + input_filename); }
	std::ofstream os(output_filename, std::ios::binary);
	if (!os) { error("Impossibile aprire il file di output " + output_filename); }

	lz4decomp(is, os);

	return EXIT_SUCCESS;
}
