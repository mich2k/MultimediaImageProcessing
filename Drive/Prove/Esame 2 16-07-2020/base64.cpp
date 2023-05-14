#include "base64.h"

class bitwriter {
    uint8_t buffer_;
    uint8_t nbits_ = 0;
    std::ostream& os_;

    void write_bit(uint8_t bit) {
        buffer_ = (buffer_ << 1) | bit;
        ++nbits_;
        // if the buffer is full (8 bits) write it to file
        if (nbits_ == 8) {
            os_.write(reinterpret_cast<char*>(&buffer_), 1);
            nbits_ = 0;
        }
    }

public:
    bitwriter(std::ostream& os) : os_(os) {}
    ~bitwriter() {
        flush();
    }

    std::ostream& write(uint32_t u, uint8_t n) {
        while (n-- > 0) {
            uint8_t bit = (u >> n) & 1;
            write_bit(bit);
        }
        return os_;
    }

    std::ostream& operator()(uint32_t u, uint8_t n) {
        return write(u, n);
    }

    void flush(uint8_t bit = 0) {
        while (nbits_ > 0) {
            write_bit(bit);
        }
    }
};

char decodeChar(char carattere) {
	if (carattere >= 'A' && carattere <= 'Z')
		return carattere - 65;
	if (carattere >= 'a' && carattere <= 'z')
		return carattere - 71;
	if (carattere >= '0' && carattere <= '9')
		return carattere + 4;
	if (carattere == '+')
		return 62;
	if (carattere == '/')
		return 63;
	return 64; // Valore non possibile --> Errore
}

std::string base64_decode(const std::string& input) {

	std::string output;

	// Controllo che la stringa non sia vuota:
	if (input.empty())
		return output;

	for (auto& carattere : input) {
		output.push_back(decodeChar(carattere));
	}

	std::string finalOutput;
	std::ostringstream os(finalOutput);
    bitwriter bw(os);
    for (size_t i = 0; i < output.length(); ++i) {
        if(output[i] != 64)
            bw.write(output[i], 6);
    }

    finalOutput = os.str();

    return finalOutput;
}
