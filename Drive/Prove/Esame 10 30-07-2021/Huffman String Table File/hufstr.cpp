#include "hufstr.h"
#include <algorithm>
#include <iterator>

struct bitwriter {

	uint32_t buffer_ = 0;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	bitwriter(std::ostream& os) : os_(os) { }
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

	void flush(uint8_t bit = 1) { // 1 padding
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}
};

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

	std::istream& read(uint32_t& u, uint8_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

uint8_t hufstr::getSym(uint32_t code) const {
	for (auto& entry : _table) {
		if (entry.second._code == code)
			return entry.first;
	}
	return 0;
}

elem hufstr::getEntry(uint32_t code) const {
	for (auto& entry : _table) {
		if (entry.second._code == code)
			return entry.second;
	}
	elem empty;
	empty._code = 0;
	empty._len = 0;
	return empty;
}

hufstr::hufstr() {
	std::ifstream is("table.bin", std::ios::binary);
	uint8_t sym;
	elem elemRead;
	while (is.read(reinterpret_cast<char*>(&sym), sizeof(sym))) {
		is.read(reinterpret_cast<char*>(&elemRead._len), sizeof(elemRead._len));
		is.read(reinterpret_cast<char*>(&elemRead._code), sizeof(elemRead._code));
		is.get();
		_table[sym] = elemRead;
		triplet t;
		t._sym = sym;
		t._len = elemRead._len;
		t._code = elemRead._code;
		_huff_table.push_back(t);
	}
}

std::vector<uint8_t> hufstr::compress(const std::string& s) const {
	std::vector<uint8_t> compressed;
	std::stringstream stream;
	bitwriter bw(stream);
	for (auto carattere : s) { // Per ogni carattere nella stringa
		uint8_t sym = (uint8_t)carattere; // Estraggo il carattere
		bw.write(_table.at(sym)._code, _table.at(sym)._len); // e riporto il simbolo sullo stream con _len bit
	}
	bw.flush(1); // 1 padding
	uint8_t chunk;
	while (stream.read(reinterpret_cast<char*>(&chunk), 1)) {
		compressed.push_back(chunk);
	}
	return compressed;
}

std::string hufstr::decompress(const std::vector<uint8_t>& v) const {
	std::stringstream stream;
	for (auto chunk : v) { // Riporto ogni elemento del vettore sul file
		stream.write(reinterpret_cast<char*>(&chunk), 1);
	}
	bitreader br(stream);
	std::string decompressedOutput;
	uint8_t tmp;
	std::vector<triplet> huff_table = _huff_table;
	sort(huff_table.begin(), huff_table.end(),
		[](const triplet& lhs, const triplet& rhs) {
			return lhs._len < rhs._len;
		});
	while (stream.read(reinterpret_cast<char*>(&tmp), 1) || br.nbits_ != 0) {
		stream.seekg((size_t)stream.tellg() - 1);  // Resetto il puntatore
		uint32_t curlen = 0;
		uint32_t curcode = 0;
		size_t pos = 0;
		while (true) {
			auto cur_entry = huff_table[pos];
			while (curlen < cur_entry._len) {
				curcode = (curcode << 1) | br.read_bit();
				++curlen;
			}
			if (curcode == cur_entry._code) {
				decompressedOutput.push_back((char)cur_entry._sym);
				break;
			}
			++pos;
			if (pos >= huff_table.size()) {
				//error("Huffman code not found");
				break;
			}
		}
	}
	return decompressedOutput;
}
