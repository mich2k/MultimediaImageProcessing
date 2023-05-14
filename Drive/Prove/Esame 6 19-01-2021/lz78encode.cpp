/*
 * Nota dell'autore: Per poter implementare l'algoritmo lz78 in maniera efficiente occorre impiegare una struttura ad albero, con conseguente
 * esplorazione ricorsiva. Con un'implementazione come quella realizzata da me, l'algoritmo entra in crisi solo nell'istanza più critica.
 * Se qualcuno se la sente, può procedere con il tree (ognuno ha i suoi gusti). Nel caso, buona fortuna.
 *
 * Emanuele Ferari
 */


#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

struct bitwriter {

	uint32_t buffer_ = 0;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	bitwriter(std::ostream& os) : os_(os) {}
	~bitwriter() {
		flush();
	}

	void write_bit(int bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

	std::ostream& write(uint32_t u, size_t n) {
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

struct lz78dictionary {

	std::vector<std::string> data_;
	int maxbits_;

	lz78dictionary(int maxbits) : maxbits_(maxbits) {}
	std::string& operator[](size_t index) { return data_[index]; }
	size_t indexOf(std::string element) {
		return (size_t)(std::find(data_.begin(), data_.end(), element) - data_.begin());
	}
	bool isInDict(std::string element) {
		if (data_.size() >= pow(2, maxbits_)) { clear(); }
		if (std::find(data_.begin(), data_.end(), element) == data_.end())
			return false;
		else return true;
	}
	void push_back_sequence(std::string sequence) { 
		if (data_.size() >= pow(2, maxbits_)) { clear(); }
		data_.push_back(sequence);
	}
	size_t size() { return data_.size(); }

private: // Utilities
	void clear() {
		data_.clear();
	}
};

void writeCouple(bitwriter& bw, uint8_t character, size_t key, size_t nbits) {
	bw.write((uint32_t)key, nbits);
	bw.write(character, 8);
}

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {

	// Command line management
	std::ifstream is(input_filename, std::ios::binary);
	if (!is) {
		std::cout << "\n\tERRORE: Impossibile aprire il file " + input_filename + "\n";
		return false;
	}
	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		std::cout << "\n\tERRORE: Impossibile aprire il file " + output_filename + "\n";
		return false;
	}
	if (maxbits < 1 || maxbits > 30) {
		std::cout << "\n\tIl parametro maxbits non rientra nell'intervallo [1, 30]\n";
		return false;
	}

	// Predispondo il bitwriter:
	os << "LZ78";
	bitwriter bw(os);
	bw.write((uint32_t)maxbits, 5);

	// Durante la lettura, occorre impiegare un dizionario
	lz78dictionary dict(maxbits);
	uint8_t chRead;
	std::string currentSequence;
	size_t lastMatch = 0;
	while (is.read(reinterpret_cast<char*>(&chRead), 1)) { // Leggo un carattere
		currentSequence.push_back(chRead); // Lo inserisco nella sequenza corrente
		if (!dict.isInDict(currentSequence)) { // Se la sequenza corrente non è presente nel dizionario
			dict.push_back_sequence(currentSequence); // la inserisco nel dizionario
			// Memorizzo l'ultimo match
			if (currentSequence.length() != 1) {
				currentSequence.pop_back();
				lastMatch = dict.indexOf(currentSequence) + 1;
			}
			else lastMatch = 0;
			// Riporto la coppia
			writeCouple(bw, chRead, lastMatch, (size_t)ceil(log2(dict.size()))); // Scrivo la sequenza, riportando l'ultimo match e il carattere letto.
			currentSequence.clear(); // ripulisco la sequenza
			lastMatch = 0; // e azzero il match
		}
	}
	// Flush dell'ultima eventuale sequenza:
	if (!currentSequence.empty()) {
		// Memorizzo l'ultimo match
		if (currentSequence.length() != 1) {
			currentSequence.pop_back();
			lastMatch = dict.indexOf(currentSequence) + 1;
		}
		else lastMatch = 0;
		writeCouple(bw, chRead, lastMatch, (size_t)ceil(log2(dict.size()))); // Scrivo la sequenza, riportando l'ultimo match e il carattere letto.
	}

	return true;
}
