#include <cstdint>
#include <vector>
#include <fstream>
#include <iterator>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1]);
	if (!is) {
		return 1;
	}

	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is),
		std::istream_iterator<int32_t>()
	};

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return 1;
	}



	// INIT SCRITTURA A BIT

	uint8_t buffer;
	int n = 0; // at first the buffer has 0 bits



	for (const auto& x : v) { // dobbiamo sostituirlo con qualcosa che scriva 11 bit alla volta
		// scriviamo un bit alla volta

		uint8_t cur_bit;	// deve essere uno degli 11 bit di x

		// i bit vengono scritti dal più significativo al meno significativo
		// quindi dovrò partire da 11, non da 0

		for (int i = 10; i >= 0; --i) {		// 11 volte scriviamo un bit in output
			cur_bit = (x >> i) & 1;	// lo sposto a dx di i-posizioni e tengo solo il bit meno sinificativo
			buffer = (buffer << 1) | cur_bit;			// lo sposto a sx di una posizione 'per fare spazio' 
														//		(l'& azzera tutto tranne il bit meno significativo)
														// mette in OR il bit corrente
			++n;										// incremento n per dire che abbiamo un bit in più
			if (n == 8) {
				raw_write(os, buffer);					// scriviamo un byte sull'output stream
				n = 0;									// così da poter scrivere altri 8 bit (prox. byte)
			}
														
		}


		raw_write(os, x);
	}

	while (n > 0) {	// finchè ci sono ancora bit nel buffer 
		uint8_t cur_bit = 0;	
		buffer = (buffer << 1) | cur_bit;		
	
		++n;										
		if (n == 8) {
			raw_write(os, buffer);
			n = 0;
		}
	}


	return 0;
}