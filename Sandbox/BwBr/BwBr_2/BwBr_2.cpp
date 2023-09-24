
#include <iostream>
#include <cstdint>
#include <fstream>
using namespace std;

template <typename T>
ostream& raw_write(ostream& os, const T& v) {
	os.write(reinterpret_cast<const char*>(&v), sizeof(T)); // reinterpret + pointer
	return os;
}

template <typename T>
istream& raw_read(istream& in, T& v) {
	in.read(reinterpret_cast<char*>(&v), sizeof(T));
	return in;
}


class bitreader {
private:
	uint8_t buffer_;
	size_t n_bits_;
	istream& in_;

public:

	bitreader(istream& in) : in_(in), n_bits_(0), buffer_(0) {};


	uint32_t read_bit() {

		// mi trovo all'inizio o a n_bits_ % 8 == 0?
		if (n_bits_ == 0) {
			// si legge sempre un byte intero di default siccome sizeof(T) è 8 (essendo a 8 bit)
			raw_read(in_, buffer_);
			n_bits_ = 8; // ripristino il byte, questa sarebbe la sizeof di quanti bit effettivi ho nel buffer, che ora è pieno
		}


		// caso comune in cui semplicemente leggo un bit DAL buffer_ e aspetto l'arrivo al byte per leggere
		// di fatto il bitreader legge i bit sempre dal buffer e poi fa una grande lettura ad ogni byte quando il buffer è vuoto

		uint32_t bit = 0;
		bit = (buffer_ >> (n_bits_ - 1)) & 1; // porto in ultima l'n_bits_ che voglio e lo isolo

		/*
			n_bits_ - 1 dato che:
				supp. x = 128 base 10 -> 1000'0000 base 2
				
				x >> 5 => 100
				x >> 6 => 10
				x >> 7 => 1

				l'1 in prima pos. può essere spostato max. di 7 posizioni
				facendo uno shift di 8 ho

				x >> 8 => 0 !!!
		
		*/

		n_bits_--;


		return bit;
	}

	istream& read(uint32_t& v, size_t n) {
		while (n-- > 0) {
			uint32_t bit = read_bit();
			v <<= 1; // libero un posto
			v |= bit; // aggiungo il bit in coda
		}

		return in_;

	}

};

class bitwriter {
private:
	uint8_t buffer_;
	size_t n_bits_;
	ostream& out_;


public:
	bitwriter(ostream& out) : out_(out), n_bits_(0), buffer_(0) {};
	~bitwriter() {
		flush();
	}

	ostream& write_bit(uint32_t bit) {
		// operazione su bit se non raggiungo il byte

		buffer_ <<= 1;                      // libero un bit sul buffer
		buffer_ |= bit;                     // lo aggiungo a buffer_
		n_bits_++;                          // incremento i bit in buffer_ PRIMA dell'if

		// se raggiungo il byte scrivo
		if (n_bits_ == 8) {
			raw_write(out_, buffer_);
			n_bits_ = 0;
			buffer_ = 0;
		}
		return out_;

	}


	ostream& write(uint32_t v, size_t n) {
		while (n-- > 0) {
			uint32_t bit = (v >> n) & 1;    // prendo l'n-esimo bit da scrivere e lo isolo
			write_bit(bit);                     // lo mando a write_bit
		}

		return out_;
	}

	void flush() {
		while (n_bits_ > 0) {
			write_bit(0);
		}
	}


};



int main(void)
{
	string fn = "f.bin";
	uint8_t n = 5;

	/* bitwriter example */
	ofstream out(fn, ios::binary);
	bitwriter bw(out);
	uint8_t c = 'a';
	uint8_t d = 'b';
	uint8_t e = 'c';
	uint8_t r = 128;

	bw.write(c, 8);
	bw.write(d, 8);
	bw.write(e, 8);
	bw.write(r, 4);


	/* bitreader example */
	//ifstream in(fn, ios::binary);
	//bitreader br(in);
	//uint32_t v = 0;
	//while (n-- > 0) {
	//    br.read(v, 2);
	//    cout << v << endl;
	//    v = 0;
	//}

}