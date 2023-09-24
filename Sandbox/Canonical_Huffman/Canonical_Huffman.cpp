
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "huffman.h"
#include <unordered_map>

class BitWriter {
private:
    uint8_t buffer_;
    uint8_t bitPos_;
    std::ostream& os;

public:
    BitWriter(std::ostream& os) : buffer_(0), bitPos_(0), os(os) {}

    void writeBit(bool bit) {
        buffer_ |= bit << (7 - bitPos_);
        bitPos_++;
        if (bitPos_ == 8) {
            os.write(reinterpret_cast<const char*>(&buffer_), 1);
            buffer_ = 0;
            bitPos_ = 0;
        }
    }

    void writeBits(uint32_t bits, uint8_t numBits) {
        for (int i = numBits - 1; i >= 0; --i) {
            writeBit((bits >> i) & 1);
        }
    }

    ~BitWriter() {
        if (bitPos_ > 0) {
            os.write(reinterpret_cast<const char*>(&buffer_), 1);
        }
    }
};

class BitReader {
private:
    uint8_t buffer_;
    uint8_t bitPos_;
    std::istream& is;

public:
    BitReader(std::istream& is) : buffer_(0), bitPos_(8), is(is) {}

    bool readBit() {
        if (bitPos_ == 8) {
            is.read(reinterpret_cast<char*>(&buffer_), 1);
            bitPos_ = 0;
        }
        bool bit = buffer_ & (1 << (7 - bitPos_));
        bitPos_++;
        return bit;
    }

    uint32_t readBits(uint8_t numBits) {
        uint32_t bits = 0;
        for (int i = numBits - 1; i >= 0; --i) {
            bits |= readBit() << i;
        }
        return bits;
    }

    bool fail() const {
        return is.fail();
    }

    explicit operator bool() const {
        return !fail();
    }
};



int main() {
    using namespace std;
    unordered_map<char, uint32_t> freq;

    freq['c'] = 24;
    freq['b'] = 15;
    freq['a'] = 10;
    freq['e'] = 9;
    freq['d'] = 2;



    huffman<char> h;

    h.create_table(freq);
    
    h.compute_canonical_codes();

    h.print_canonical_codes();

    return EXIT_SUCCESS;
}