#include "lzs.h"

struct bitreader {
	uint32_t buffer_ = 0;
	uint32_t nbits_ = 0;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {}

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
	bool operator!() { return !is_; }

	std::istream& read(uint32_t& u, uint32_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

struct bitwriter {

	uint32_t buffer_ = 0;
	uint32_t nbits_ = 0;
	std::ostream& os_;

	bitwriter(std::ostream& os) : os_(os) {}
	~bitwriter() { flush(); }

	void write_bit(int bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

	std::ostream& write(uint32_t u, uint8_t n) {
		while (n-- > 0) {
			uint8_t bit = (u >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}
};

uint32_t readLength(bitreader& br) {
	uint32_t firstTwoBits;
	br.read(firstTwoBits, 2);
	if (firstTwoBits == 0b00) { return 2; }
	if (firstTwoBits == 0b01) { return 3; }
	if (firstTwoBits == 0b10) { return 4; }
	if (firstTwoBits == 0b11) {
		uint32_t secondTwoBits;
		br.read(secondTwoBits, 2);
		if (secondTwoBits == 0b00) { return 5; }
		if (secondTwoBits == 0b01) { return 6; }
		if (secondTwoBits == 0b10) { return 7; }
		uint32_t fourBits;
		uint32_t N = 1;
		while (true) {
			br.read(fourBits, 4); // Leggo i quattro bit successivi
			if (fourBits != 0b1111) // Se ho una sequenza differente da 1111
				break; // mi fermo
			++N; // altrimenti incremento il contatore
		}
		return fourBits + (N * 15 - 7);
	}
	return 0;
}

void writeLength(bitwriter& bw, uint32_t length) {
	if (length == 2) { bw.write(0b00, 2); }
	if (length == 3) { bw.write(0b01, 2); }
	if (length == 4) { bw.write(0b10, 2); }
	if (length == 5) { bw.write(0b1100, 4); }
	if (length == 6) { bw.write(0b1101, 4); }
	if (length == 7) { bw.write(0b1110, 4); }
	if (length > 7) {
		uint32_t N = (length + 7) / 15;
		for (uint32_t i = 0; i < N; ++i) {
			bw.write(0b1111, 4);
		}
		bw.write(length - (N * 15 - 7), 4);
	}
}

struct sliding_window_dictionary {
	std::string content;
	char& operator[](uint32_t index) {
		return content[index];
	}
	void append(std::string str) {
		content.append(str);
		if (content.length() > 2048)
			content.erase(0, content.length() - 2048);
	}
	std::string getSequence(uint32_t offset, uint32_t length) {
		while (offset > content.length()) {
			offset -= content.length();
		}
		if (length > offset) {
			length -= offset;
			std::string sequence = content.substr(content.length() - offset, content.length() + offset - 1);
			char toRepeat = sequence.back();
			while (length-- > 0)
				sequence.push_back(toRepeat);
			return sequence;
		}
		return content.substr(content.length() - offset, length);
	}
	bool contains(std::string str) {
		if (content.find(str) != std::string::npos)
			return true;
		return false;
	}
	uint32_t getOffset(std::string str) {
		uint32_t index = content.find(str);
		return str.length() - index + 1;
	}
};

void lzs_decompress(std::istream& is, std::ostream& os) {

	uint32_t bit;
	bitreader br(is);
	std::string decodedSequence;
	sliding_window_dictionary dict;
	while (br.read(bit, 1)) {
		if (bit == 0) { // Literal Byte
			uint32_t byte;
			br.read(byte, 8); // leggo il byte
			decodedSequence.push_back((char)byte); // e lo riporto sulla sequenza decodificata
		}
		else { // (bit == 1), Offset / length
			uint32_t offset, length;
			br.read(bit, 1); // Leggo l'identificatore dell'offset
			if (bit == 1) { // se è minore di 128
				br.read(offset, 7); // leggo 7 bit di offset value
				if (offset == 0b0000000) break; // End marker
			}
			else { // Altrimenti
				br.read(offset, 11); // leggo 11 bit di offset value
			}
			length = readLength(br); // Leggo la length
			decodedSequence = dict.getSequence(offset, length); // e riporto la sequenza estratta dal dizionario
		}
		os << decodedSequence; // Riporto la sequenza decodificata sul file di output
		dict.append(decodedSequence); // e aggiorno il dizionario
		decodedSequence.clear();
	}
}

void lzs_compress(std::istream& is, std::ostream& os) {
	uint8_t chRead;
	std::string currentSequence;
	sliding_window_dictionary dict;
	bitwriter bw(os);
	while (is.read(reinterpret_cast<char*>(&chRead), 1)) {
		currentSequence.push_back((char)chRead); // Aggiungo il carattere alla sequenza corrente
		if (!dict.contains(currentSequence)) { // se non è presente nel dizionario
			if (currentSequence.length() == 1) { // se la lunghezza è 1
				bw.write(0, 1); // riporto uno 0
				bw.write(chRead, 8); // seguito dal carattere considerato
			}
			else { // Altrimenti
				std::string lastMatch = currentSequence; // Memorizzo l'ultimo match
				lastMatch.pop_back(); // eliminando l'ultimo carattere
				bw.write(1, 1); // riporto un 1
				if (currentSequence.length() < 128) { // Se la sequenza è lunga meno di 128 caratteri
					bw.write(1, 1); // riporto un 1
					bw.write(dict.getOffset(lastMatch), 7); // seguito dall'offset riportato con 7 bit
				}
				else { // altrimenti
					bw.write(0, 1); // Riporto uno 0
					bw.write(dict.getOffset(lastMatch), 11); // seguito dall'offset riportato con 11 bit
				}
				writeLength(bw, currentSequence.length()); // e riporto la lunghezza
			}
			dict.append(currentSequence); // Aggiungo la sequenza corrente al dizionario
			currentSequence.clear();
		}
	}
	bw.write(0b110000000, 9);
}