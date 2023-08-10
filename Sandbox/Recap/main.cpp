#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>


using namespace std;

template <typename T>
ostream& raw_write(ostream& os,const T& v) {
	os.write(reinterpret_cast<const char*>(&v), sizeof(T));
	return os;
}

template <typename T>
istream& raw_read(istream& in, T& v) {
	in.read(reinterpret_cast<char*>(&v), sizeof(T));
	return in;
}

class bitwriter {
private:
	size_t n_bits_;
	uint8_t buffer_;
	ostream& os_;

public:
	bitwriter(ostream& os) : n_bits_(0), buffer_(0), os_(os) {};

	void write_bit(uint8_t bit) {
		buffer_ <<= 1;
		buffer_ |= (bit & 1);
		n_bits_++;
		if (n_bits_ == 8) {
			raw_write(os_, buffer_);
			n_bits_ = 0;
			buffer_ = 0;
		}

	}

	void write(uint32_t u, size_t n) {

		while (n-- > 0) {
			uint32_t bit = (u >> n) & 1;
			write_bit(bit);
		}

	}

	~bitwriter(){
		while (n_bits_ > 0) {
			write_bit(0);
			}
	};

};


class bitreader {
private:
	istream& in_;
	uint8_t buffer_;
	uint32_t n_bits_;

public:
	bitreader(istream& in) : in_(in), n_bits_(0), buffer_(0) {};


	istream& read_bit(uint32_t& v) {

		if (n_bits_ == 0) {
			raw_read(in_, buffer_);
			n_bits_ = 8;
		}
		uint8_t bit = (buffer_ >> (n_bits_ - 1)) & 1;
		v <<= 1;
		v |= bit;
		n_bits_--;

		return in_;
	}

	istream& read(uint32_t& v, size_t n){
	
		while (n-- > 0) {
			read_bit(v);
		}

		return in_;
	}

};



int main(void) {

	//ofstream os("out.bin", ios::binary);
	ifstream in("out.bin", ios::binary);
	//bitwriter bw(os);
	bitreader br(in);

	uint32_t x = 0;

	// bw.write(2, 2);
	// bw.~bitwriter();
	br.read(x, 2);
	cout << x << endl;




	return EXIT_SUCCESS;
}