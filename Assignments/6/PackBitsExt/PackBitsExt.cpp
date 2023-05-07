#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <streambuf>
#include <deque>


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


int rle_run(uint8_t c, uint8_t n, BitWriter& bw) {
    uint16_t v = 257;
    v -= n;
    bw.writeBits(v, 8);
    bw.writeBits(c, 8);

    return 0;
}

template<typename container>
uint8_t get_run_index(uint8_t c, const container& Container, bool ext=false, uint8_t* including_doubles_cont=NULL) {
    container v = Container;
    uint8_t index = 0;
    bool flag = false;
    for (auto e : v) {
        if (e == c && index < 128) {
            if (including_doubles_cont) {
                *(including_doubles_cont)+=1;
            }
            index++;
            flag = true;
        }
        else {
            if (ext && (e == c || e == c+1 ) && index < 128) {
                index++;
                if (including_doubles_cont) {
                    (*including_doubles_cont) += 2;
                }
                flag = true;
            }
            else {
                if (flag) {
                    return index;
                }
            }

        }
    }
    return index;
}


int coding(std::string& filename, std::string& out_filename) {
    using namespace std;
    using std::cout;
    ofstream out{ out_filename, ios::binary };
    if (out.fail())
        return 1;
    ifstream in{ filename, ios::binary };
    if (in.fail())
        return 1;

    if (!in || !out) {
        return 1;
    }
    BitWriter bw{ out };
    BitReader br{ in };
    vector<uint8_t> control(0);
    char e = 0, curr = 0;
    char past = 0;
    size_t count = 0;

    // .get does not avoid ws
    // >> consumes ws!

    while (in.get(e)) {
        // cout << e;
        count++;
    }
    in.clear();
    in.seekg(0);
    // cout << endl;

    bool flag = false;

    deque<uint8_t> copy_buffer;

    deque<uint8_t> index_q;
    uint8_t index = 0;
    for (size_t k = 0; k < count; k += index) {

        //uint8_t past = br.readBits(8);
        in.get(past); // !!!!!!
        if (in.fail() || in.eof()) {
            break;
        }
        if (!flag) {
            in.seekg(0);
        }
        flag = true;
        index = 0;
        streamoff pos = in.tellg();
        char c = 0;
        uint8_t first_c = '\0';
        for (size_t i = 0; i < 128; i++) {
            in.get(c);  // !!!!!
            if (in.fail() || in.eof()) {
                break;
            }

            if (i == 0) {
                first_c = c;
                copy_buffer.push_back(first_c);
            }

            control.push_back(c);

        }

        index = get_run_index(first_c, control);
        if (index) {
            index_q.push_back(index);
            // cout << (uint32_t)index;
        }


        // cout << endl;
        if (control.size() > 0) {
            in.clear();

        }
        control.clear();

        in.seekg(pos + (index - 1));

    }

    // cout << endl << "init queue" << endl;

    while (index_q.size() > 0) {
        if (index_q.front() > 2) {

            rle_run(copy_buffer.front(), index_q.front(), bw);
            index_q.pop_front();
            copy_buffer.pop_front();

            continue;
        }else{
            uint8_t including_doubles_cont = 0;

            uint8_t cont = get_run_index(1, index_q, true, &including_doubles_cont);
            // cout << (uint32_t) cont << endl;
            bw.writeBits(including_doubles_cont - 1, 8 );

            for (uint8_t r = 0; r < cont; r++) {
                for (uint8_t n = 0; n < index_q.front(); ++n) {
                    bw.writeBits(copy_buffer.front(), 8);
                }
                copy_buffer.pop_front();
                index_q.pop_front();
            }

            // index_q.erase(index_q.begin(), index_q.begin() + cont);
        }
    }

    bw.writeBits(128, 8);


    return EXIT_SUCCESS;
}


int decode(std::string& filename, std::string& out_filename) {
    using namespace std;
    using std::cout;
    ifstream in(filename, ios::binary);
    if (in.fail())
        return 1;
    ofstream out(out_filename, ios::binary);
    if (out.fail())
        return 1;


    if (!in || !out) {
        return 1;
    }
    BitReader br{ in };
    BitWriter bw(out);

    while (true) {
        if (br.fail()) {
            break;
        }
        uint8_t L = br.readBits(8);
        uint8_t c = 0;
        if (L >= 0 && L <= 127) {
            for (uint8_t i = 0; i < L + 1; ++i) {
                c = br.readBits(8);
                out << c;
                //bw.writeBits(c, 8);
            }
        }
        else {
            if (L >= 129 && L <= 255) {
                c = br.readBits(8);
                for (uint8_t i = 0; i < 257 - L; ++i) {
                    out << c;
                    //bw.writeBits(c, 8);

                }
            }
            else {
                if (L == 128) {
                    cout << endl << "End Decoding" << endl;
                    break;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}


int main(int argc, char** argv)
{
    using namespace std;
    string in_filename = string(argv[2]);
    string out_filename = string(argv[3]);



    int RET = *argv[1] == 'c' ? coding(in_filename, out_filename) : *argv[1] == 'd' ? decode(in_filename, out_filename) : -1;

    return RET;
}

