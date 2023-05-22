// 14_Z85rot.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <ctype.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <deque>

using namespace std;

using rgb = array<uint8_t, 3>;

template <typename T>
istream& raw_read(istream& in, T& v, size_t n) {
    return in.read(reinterpret_cast<char*>(&v), n);
}

template <typename T>
struct mat {
    int rows_;
    int cols_;
    vector<T> data_;

    mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows*cols) {};

    // grazie alla T& reference posso assegnare un valore
    // questa def. mi permette di
    //    -> assegnare
    //    -> leggere
    T& operator()(int r, int c) {
        return data_[cols_ * r + c];
    }

    char* raw_data() {
        return reinterpret_cast<char*>(&data_[0]);
    }

    size_t raw_size() {
        return rows_ * cols_ * sizeof(T);
    }


};

bool check(string s, ifstream& in) {
    bool cmtFound = false;
    if (s != "#")
        return false;
    char c = 0;
    while (c != 0x0A) {
        in.get(c);
    }
    return true;
}

mat<rgb>* load_ppm(const char* fn) {

    ifstream in(fn, ios::binary);
    if (in.fail())
        return NULL;
    string mw, W, H, maxVal;
    char c;

    in >> mw;
    if (mw != "P6")
        return NULL;
    do {
        in >> W;
    } while (check(W, in));

    do {
        in >> H;
    } while (check(H, in));


    if (!(stoi(W) > 0 && stoi(H) > 0))
        return NULL;

    do {
        in >> maxVal;
    } while (check(maxVal, in));

    if (stoi(maxVal) > 65536 && stoi(maxVal) < 0)
        return NULL;

    in.ignore(); // last ws

    mat<rgb>* m = new mat<rgb>(stoi(H), stoi(W));

    // C-Like
    // raw_read(in, m->data_[0], m->cols_ * m->rows_ * sizeof(rgb));
    
    raw_read(in, *m->raw_data(), m->raw_size());

    return m;
}


class base85 {
private:
    vector<uint8_t> data_;
    vector<uint32_t> raw_bins_;
    vector<uint8_t> encoded_;
    size_t count_, N_, actualRotation_;
    unordered_map<uint8_t, uint8_t> encode_table_;
    mat<rgb>* m_;
    const char* output_filename_;
    bool paddingNeeded_ = false;
    
    void decToBase(uint32_t in) {
        vector<uint8_t> v;
        while (true) {
            if (in == 0 && v.size() == 5)
                break;
            if (v.size() < 5 && in == 0) {
                v.push_back(0);
                continue;
            }
            uint32_t r = in % 85;
            in /= 85;
            v.push_back(r);
        }
        count_ += v.size();

        for (auto it = v.rbegin(); it != v.rend(); it++) {
            data_.push_back(*it);
        }

    }


public:
    base85(mat<rgb>* m, size_t N, const char* out_fn): count_(0), m_(m), N_(N), actualRotation_(0), output_filename_(out_fn) {
        uint8_t i = 0;
        array<uint8_t, 24> sym = { '.', '-', ':',
            '+' ,'=','^', '!' ,'/','*' ,'?' ,'&',
            '<' ,'>' ,'(',')' ,'[',']' ,'{',
        '}', '@', '%', '$' ,'#' };

        for (; i <= 9; i++)
            encode_table_[i] = i;
        for (; i < 36; i++)
            encode_table_[i] = i + 87;
        for (; i < 62; i++)
            encode_table_[i] = i + 29;
        for (; i <= 85; i++)
            encode_table_[i] = sym[i - 62];
        //for (auto e : encode_table_)
        //    cout << e.second << " " << e.first + '0' << endl;

        if ((m->rows_ * m->cols_) % 4 != 0)
            paddingNeeded_ = true;
    };


    size_t getRotatedIndex(size_t N, uint8_t e) {
        int index = e - actualRotation_;
        index = index < 0 ? index += (encode_table_.size() - 1) : index;
        actualRotation_ += N_;
        return index;
    }

    void buildDecVector() {
        for (auto e : raw_bins_)
            decToBase(e);

    }

    void buildRawDecBin() {
        uint32_t dec = 0;
        deque<uint8_t> v;

        if (!paddingNeeded_) {
            for (size_t r = 0; r < m_->rows_; r++) {
                for (size_t c = 0; c < m_->cols_; c++) {
                    v.push_back((*m_)(r, c)[0]);
                    v.push_back((*m_)(r, c)[1]);
                    v.push_back((*m_)(r, c)[2]);

                    if (v.size() >= 4) {
                        
                        uint32_t x = 0;
                        // byte più significativo 
                        dec |= v[0];
                        dec <<= 24;

                        // 2 byte
                        x |= v[1];
                        x <<= 16;
                        dec += x;
                        x = 0;

                        // 3 byte
                        x |= v[2];
                        x <<= 8;
                        dec += x;

                        // 4 byte: non serve shiftarlo
                        dec += v[3];
                        raw_bins_.push_back(dec);
                        dec = 0;
                        for (size_t i = 0; i < 4; i++)
                            v.pop_front();

                    }
                }
            }
        }
        else {
            cout << "Needs padding!" << endl;
        }

    }

    void encodeDecVector() {
        for (auto e : data_) {
            size_t index = getRotatedIndex(N_, e);
            encoded_.push_back(encode_table_[index]);
        }
    }


    void printEncodedVector() {
        for (auto e : encoded_)
            cout << e;
    }


    bool writeEncodeToFile() {
        ofstream out(output_filename_, ios::binary);
        out << m_->cols_;
        out << ',';
        out << m_->rows_;
        out << ',';
        for (auto e : encoded_)
            out << e;

        if (out.fail())
            return false;
        
        return true;
    }

};


int main(int argc, char** argv)
{
    if (argc != 5)
        return EXIT_FAILURE;


    switch (*argv[1]) {
    case 'c': {
        mat<rgb>* m = load_ppm(argv[3]);
        if (m == NULL)
            return EXIT_FAILURE;



        size_t N = 1;
        base85 b(m, atoi(argv[2]), argv[4]);
        b.buildRawDecBin();
        b.buildDecVector();
        b.encodeDecVector();
        b.writeEncodeToFile();
        break;
    }

    case 'd':
        cout << "Decode not avialable" << endl;
        break;

    default:
        cout << "Wrong usage" << endl;
        return EXIT_FAILURE;


    }

    
}