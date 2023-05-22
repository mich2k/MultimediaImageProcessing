// BwBr.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
using namespace std;

// entrambi vogliono ISTREAM ed OSTREAM, senza F, sarebbe la classe padre


// il write prende const e, anche nel cast siccome non lo modifica
template <typename T>
ostream& raw_write(ostream& out, const T& e) {  // referenza
    return out.write(reinterpret_cast<const char*>(&e), sizeof(T)); // puntatore alla referenza o alla copia
}

// il read NON ha nulla di const, siccome DEVE alterare il valore
template <typename T>
istream& raw_read(istream& in, T& n) {
    return in.read(reinterpret_cast<char*>(&n), sizeof(T));
}




class gbitwriter {
	std::ostream& os_;
	uint8_t buffer_;
	size_t nbits_;

	std::ostream& write_bit(uint32_t u) {
		// buffer_ = buffer_ * 2 + u % 2;
		buffer_ = (buffer_ << 1) | (u & 1);
		++nbits_;
		if (nbits_ == 8) {
			raw_write(os_, buffer_);
			nbits_ = 0;
		}
		return os_;
	}

public:
	gbitwriter(std::ostream& os) : os_(os), nbits_(0) {}

	~gbitwriter() {
		flush();
	}

	std::ostream& write(uint32_t u, size_t n) {
		while (n-- > 0) {
			write_bit(u >> n);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, size_t n) {
		return write(u, n);
	}

	void flush(uint32_t u = 0) {
		while (nbits_ > 0) {
			write_bit(u);
		}
	}
};


struct bitwriter {

    ostream& out_;
    uint8_t buffer_ = 0;
    size_t n_bits_ = 0;


	// ostream posto nell'init.
    bitwriter(ostream& out) : out_(out) {
        n_bits_ = 0;
    }

    ostream& write_bit(uint32_t v) {

		buffer_ <<= 1;	// shifto indietro per liberare una posizione
		buffer_ |= (v & 1);	// aggiungo il bit mascherato nell'ultima pos. appena liberata
		n_bits_++;
		if (n_bits_ == 8) {
            raw_write(out_, buffer_);
            n_bits_ = 0;
        }
        return out_;
    }

    ostream& write(uint32_t v, size_t n) {
        while (n-- > 0) {
            write_bit(v >> n);
        }
        return out_;
    }

	~bitwriter() {
		flush();
	}

	void flush() {
		while (n_bits_ > 0) {
			write_bit(0);
		}
	}


};

struct bitreader {
	uint8_t buffer_;
	istream& in_;
	size_t n_bits_;

	bitreader(istream& in) : in_(in) {
		n_bits_ = 0;
	};


	uint32_t read_bit() {
		if (n_bits_ == 0) {
			raw_read(in_, buffer_);
			n_bits_ = 8;
		}
		n_bits_--;
		return (buffer_ >> n_bits_) & 1;
	}

	istream& read(uint32_t& v, size_t n) {
		
		while (n-- > 0) {
			v <<= 1;
			v |= read_bit();

		}
		return in_;
	}



};



int main(){
    //ofstream out("ouat.bin", ios::binary);
	uint32_t v = 6, v2 = 7, vr=0;
    //bitwriter bw(out);
	ifstream in("ouat.bin");
	bitreader br(in);
	br.read(vr, 2);
	cout << vr << endl;
	vr = 0;
	//br.read(vr, 3);
	//cout << vr << endl;
	//vr = 0;

	//br.read(vr, 3);
	//cout << vr << endl;


	//bw.write(v, 3);
	//bw.write(v, 3);
	//bw.write(v2, 3);
}