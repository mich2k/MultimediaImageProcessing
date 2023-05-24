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
#include <cmath>

using namespace std;

using rgb = array<uint8_t, 3>;


// problema

// da m->data_[74810] fino alla fine contengono solo
// array<uint8_t,3> = {0,0,0} ovvero tutti neri, ancor prima della processione
// problema nel load?

template <typename T>
struct mat {
    int rows_;
    int cols_;
    vector<T> data_;

    mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols) {};

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


// unico metodo rubato

bool save_ppm(const std::string& filename, mat<rgb>* img, bool ascii)
{
    std::ofstream os(filename, std::ios::binary);
    if (!os)
        return false;
    os << (ascii ? "P3" : "P6") << "\n";
    os << "# MDP2020\n";
    os << img->cols_ << " " << img->rows_ << "\n";
    os << "255\n";
    
    if (ascii) {
        for (int r = 0; r < img->rows_; ++r) {
            for (int c = 0; c < img->cols_; ++c) {
                os << static_cast<int>(img->operator()(r, c)[0]) << " "
                    << static_cast<int>(img->operator()(r, c)[1]) << " "
                    << static_cast<int>(img->operator()(r, c)[2]) << " ";
            }
            os << "\n";
        }
    }
    else {
        os.write(img->raw_data(), img->cols_ * img->rows_ * sizeof(rgb));
    }

    return true;
}

// fine furto


template <typename T>
istream& raw_read(istream& in, T& v, size_t n) {
    return in.read(reinterpret_cast<char*>(&v), n);
}

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


static uint8_t get_byte(std::istream& is)
{
    int val;
    is >> val;
    if (0 <= val && val <= 255)
        return val;
    else
        throw std::range_error("Wrong pixel value");
}

mat<rgb>* load_ppm(const char* fn) {

    ifstream in(fn, ios::binary);
    if (in.fail())
        return NULL;
    string mw, W, H, maxVal;
    char c;

    in >> mw;
    bool ascii = false;
    if (mw != "P6" && mw!="P3")
        return NULL;
    if (mw == "P3")
        ascii = true;
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
    

    if (ascii) {
        for (int r = 0; r < m->rows_; ++r) {
            for (int c = 0; c < m->cols_; ++c) {
                try {
                    //int val;
                    //in >> val;
                    m->operator()(r, c)[0] = get_byte(in);
                    m->operator()(r, c)[1] = get_byte(in);
                    m->operator()(r, c)[2] = get_byte(in);
                }
                catch (const exception& e) {
                    std::cout << e.what() << "\n";
                    return NULL;
                };
            }
        }
    }
    else {
        raw_read(in, *m->raw_data(), m->raw_size());

    }



    return m;
}


struct base85 {
    vector<uint8_t> data_;
    vector<uint32_t> raw_bins_;
    vector<uint8_t> encoded_;
    size_t count_, N_, actualRotation_;
    unordered_map<uint8_t, uint8_t> encode_table_;
    const char* output_filename_;
    bool paddingNeeded_ = false;
    mat<rgb>* m_;

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

        if (data_.size() == 288555) {
            cout << data_.size();
        }

        for (auto it = v.rbegin(); it != v.rend(); it++) {
            data_.push_back(*it);
        }

    }

    base85(size_t N, const char* out_fn) : N_(N), output_filename_(out_fn), actualRotation_(0){
    
        buildEncodeTable();
    };
    base85(mat<rgb>* m, size_t N, const char* out_fn): count_(0), m_(m), N_(N), actualRotation_(0), output_filename_(out_fn) {
        
        buildEncodeTable();
        if ((m->rows_ * m->cols_) % 4 != 0)
            paddingNeeded_ = true;
    };


    void buildEncodeTable() {
        uint8_t i = 0;
        array<uint8_t, 24> sym = { '.', '-', ':',
            '+' ,'=','^', '!' ,'/','*' ,'?' ,'&',
            '<' ,'>' ,'(',')' ,'[',']' ,'{',
        '}', '@', '%', '$' ,'#' };

        for (; i <= 9; i++)
            encode_table_[i] = i + '0';
        for (; i < 36; i++)
            encode_table_[i] = i + 87;
        for (; i < 62; i++)
            encode_table_[i] = i + 29;
        for (; i <= 85; i++)
            encode_table_[i] = sym[i - 62];
        //for (auto e : encode_table_)
        //    cout << e.second << " " << e.first + '0' << endl;


    }


    size_t getRotatedIndex(uint8_t e) {
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
        size_t i = 0;
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

                        i += 1;
                        // raw_bins ha solo zeri da 56'111

                    }
                }
            }

            if (paddingNeeded_ && v.size() > 0) {
                size_t diff = 4 - v.size();
                uint8_t bits = 24;
                uint32_t dec = 0, x = 0;

                dec |= v[0];
                dec <<= bits;
                bits -= 8;
                v.pop_front();
                for (auto e: v) {
                    x = e; // e uint8_t non è possibile shiftarci
                    x <<= bits;
                    dec |= x;
                    bits -= 8;
                }

                // gli altri diff gruppi di 8 bit sono automaticamente a zero

                raw_bins_.push_back(dec);

            }
    }

    void encodeDecVector() {
        size_t i = 0;
        for (auto e : data_) {
            size_t index = getRotatedIndex(e);
            i++;
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

        cout << endl << "Codificati: " << encoded_.size() << endl;
        cout << "IMG Size: " << m_->raw_size() << " " << sizeof(rgb);


        size_t i = 0;
        pair<size_t, size_t> p = make_pair<size_t, size_t>(0, 0);

        // DA 280556 ho tutti zeri

        for (auto e : encoded_) {
            out << e;
            if (e == '0') { 
                p.first++;
                if ((p.second + 1) == i) {
                    cout << "possible loop at " << i << endl;
                }
                p.second = i;


            }
            ++i;
        }
        cout << "Zeros: " << p.first;

        if (out.fail())
            return false;
        
        return true;
    }

    // bug: la deque vars va più avanti rispetto r,c
    // avendo messo un break al fail, anche se rilevato comunque l'immagine non viene completata siccome vars contiene ancora dati
        // fix: devo fermarmi quando termina il file bin e flushare la deque
        // alternative fix: devo fermarmi quando r e c raggiungono il valore letto e non in condizione true+break





    void decode(const char* in_filename) {
        ifstream in(in_filename, ios::binary);
        uint32_t H, W;
        in >> W;
        char x = in.get();
        in >> H;
        x = in.get();

        mat<rgb>* img = new mat<rgb>(H,W);
        m_ = img;
        int r = 0, c = 0;
        size_t avial_vars = 0;
        deque<uint8_t> vars, elements;
        vector<uint8_t> v;
        size_t pix_count = 0;
        int i_elements = 0;






        bool flag = false;
        while (true) {

            if (in.fail() || in.eof())
                break;

            uint32_t dec = 0;
            for (size_t i = 0; i < 5; i++) {
                char c = in.get();
                if (in.fail() || in.eof()) {
                    flag = true;
                    break;
                }
                for (auto& e : encode_table_) {
                    if (e.second == c) {
                        elements.push_back(getRotatedIndex(e.first));

                    }
                }
            }

                dec = 0;

                for (int j = 4, i = 0; elements.size() != 0; i++, j--) {
                    dec += elements.front() * pow(85, j);
                    elements.pop_front();
                }

                uint8_t f = 0, s = 0, t = 0, fou = 0;

                f = (dec >> 24);
                s = (dec >> 16);
                t = (dec >> 8);
                unsigned mask = (1 << 8); // ottengo maschera a 8 bit: 0001'0000'0000 (256)
                mask -= 1;               // maschera: 1111'1111                       (255)
                fou = dec & mask; // applico maschera

                vars.push_back(f);
                vars.push_back(s);
                vars.push_back(t);
                vars.push_back(fou);
            

                if (vars.size() >= 3) {
                    m_->operator()(r, c)[0] = vars.front();
                    vars.pop_front();
                    m_->operator()(r, c)[1] = vars.front();
                    vars.pop_front();
                    m_->operator()(r, c)[2] = vars.front();
                    vars.pop_front();

                }

                c++;
                if (c == m_->cols_) {
                    r++;
                    c = 0;
                }


        }


        // flush deque

        while (vars.size() > 1) {
            m_->operator()(r, c)[0] = vars.front();
            vars.pop_front();
            m_->operator()(r, c)[1] = vars.front();
            vars.pop_front();
            m_->operator()(r, c)[2] = vars.front();
            vars.pop_front();
        }


        save_ppm(output_filename_, m_, 0);
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
        //b.printEncodedVector();
        b.writeEncodeToFile();
        break;
    }

    case 'd': {
        base85 b(*argv[2] - '0', argv[4]);
        b.decode(argv[3]);


        break;

    }

    default:
        cout << "Wrong usage" << endl;
        return EXIT_FAILURE;


    }

    
}