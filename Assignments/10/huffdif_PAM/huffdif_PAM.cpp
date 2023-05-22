// huffdif_PAM.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <array>
#include <cassert>
#include <fstream>
#include "huffman.h"
#include <unordered_map>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}


class bitwriter {
	uint8_t buffer_;
	int n_ = 0;
	std::ostream& os_;

	std::ostream& write_bit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;
		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os) {}

	std::ostream& write(uint32_t u, uint8_t n) {
		//while (n --> 0) {
		//  write_bit(u >> n);
		//}
		for (int i = n - 1; i >= 0; --i) {
			write_bit(u >> i);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, uint8_t n) {
		return write(u, n);
	}

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			write_bit(bit);
		}
		return os_;
	}

	~bitwriter() {
		flush();
	}
};

class bitreader {
	uint8_t buffer_;
	uint8_t n_ = 0;
	std::istream& is_;

public:
	bitreader(std::istream& is) : is_(is) {}

	uint32_t read_bit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	uint32_t read(uint8_t n) {
		uint32_t u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return u;
	}

	uint32_t operator()(uint8_t n) {
		return read(n);
	}

	bool fail() const {
		return is_.fail();
	}

	explicit operator bool() const {
		return !fail();
	}
};


template <typename T>
struct mat {
	int rows_, cols_;
	std::vector<T> data_;

	mat(int rows = 0, int cols = 0) : rows_(rows), cols_(cols), data_(rows* cols) {}

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	const T& at(int r, int c) const {
		assert(r >= 0 && r < rows_ && c >= 0 && c < cols_);
		return data_[r * cols_ + c];
	}
	T& at(int r, int c) {
		return const_cast<T&>(static_cast<const mat*>(this)->at(r, c));
	}

	const T& operator()(int r, int c) const {
		assert(r >= 0 && r < rows_ && c >= 0 && c < cols_);
		return data_[r * cols_ + c];
	}
	T& operator()(int r, int c) {
		assert(r >= 0 && r < rows_ && c >= 0 && c < cols_);
		return data_[r * cols_ + c];
	}

	int rows() const { return rows_; }
	int cols() const { return cols_; }
	int size() const { return rows_ * cols_; }

	size_t raw_size() const {
		return rows_ * cols_ * sizeof(T);
	}
	const char* raw_data() const {
		return reinterpret_cast<const char*>(&data_[0]);
	}
};


// grayscale save pam
bool save_pam(const mat<uint8_t>& img, const std::string& filename)
{
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}
	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 1\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE GRAYSCALE\n";
	os << "ENDHDR\n";

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			os.put(img(r, c));
		}
	}

	//os.write(img.raw_data(), img.raw_size());

	return true;
}

// grayscale load pam
bool load_pam(mat<uint8_t>& img, const std::string& filename)
{
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string magic_number;
	std::getline(is, magic_number);
	if (magic_number != "P7") {
		return false;
	}

	int w, h;
	while (1) {
		std::string line;
		std::getline(is, line);
		if (line == "ENDHDR") {
			break;
		}
		std::stringstream ss(line);
		std::string token;
		ss >> token;
		if (token == "WIDTH") {
			ss >> w;
		}
		else if (token == "HEIGHT") {
			ss >> h;
		}
		else if (token == "DEPTH") {
			int depth;
			ss >> depth;
			if (depth != 1) {
				return false;
			}
		}
		else if (token == "MAXVAL") {
			int maxval;
			ss >> maxval;
			if (maxval != 255) {
				return false;
			}
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype;
			ss >> tupltype;
			if (tupltype != "GRAYSCALE") {
				return false;
			}
		}
		else {
			return false;
		}
	}

	img.resize(h, w);

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img(r, c) = is.get();
		}
	}

	return true;
}



mat<int> calc_diffm(mat<uint8_t>& m) {

	mat<int> diff;
	diff.resize(m.rows(), m.cols());

	int prev = 0;

	for (int r = 0; r < m.rows(); ++r) {
		for (int c = 0; c < m.cols(); ++c) {
			diff(r, c) = m(r, c) - prev;
			prev = m(r, c);
		}
		prev = m(r, 0);
	}


	//for (size_t r = 0; r < diff.rows(); ++r) {
	//	for (size_t c = 0; c < diff.cols(); ++c) {


	//		for (int c = 0; c < diff.cols(); ++c) {
	//			diff(r, c) = mdiff(r, c) - prev;
	//			prev = img(r, c);
	//		}
	//		prev = img(r, 0);
	//		//if (r == 0 && c == 0) {
	//		//	mdiff(r, c) = m(r, c);
	//		//	continue;
	//		//}
	//		//if (r == 0 && c > 0) {
	//		//	mdiff(r, c) = m(0, c) - m(0, c - 1);
	//		//	continue;
	//		//}
	//		//if (r > 0) {
	//		//	mdiff(r, c) = m(r, c) - m(r - 1, c);
	//		//}

	//	}
	//}

	return diff;
}

mat<uint8_t> calc_viewable_diffm(mat<int>& diffm) {
	mat<uint8_t> v(diffm.rows(), diffm.cols());

	for (size_t r = 0; r < diffm.rows(); r++) {
		for (size_t c = 0; c < diffm.cols(); c++) {
			int rc = diffm(r,c);

			if (rc == 0) {
				v(r, c) = 128;
				continue;
			}
				
			if (rc < -127) {
				v(r,c) = 0;
				continue;
			}

			if (rc > 127) {
				v(r,c) = 255;
				continue;
			}
			
			rc += 128;

			v(r, c) = static_cast<uint8_t>(rc);
		}
	}

	return v;
}

bool generate_huffdiff(mat<int> mdiff, std::string out_filename, huffman<int>& h) {

	using namespace std;
	ofstream os(out_filename, ios::binary);
	bitwriter bw(os);
	os << "HUFFDIFF";
	uint32_t c = mdiff.cols();
	uint32_t r = mdiff.rows();
	raw_write(os, c);
	raw_write(os, r);

	// scrivo N Table Entries
	uint32_t size = h.codes_table_.size();

	bw.write(size, 9);

	// scrivo le Table Entries
	for (const auto& e : h.codes_table_) {
		bw.write(e.sym_, 9);
		bw.write(e.len_, 5);
	}

	// codifico
	// possibile utilizzo search_map invece di arrivare a O(n^3) nel worst case
	for (size_t r = 0; r < mdiff.rows(); r++) {
		for (size_t c = 0; c < mdiff.cols(); c++) {
			uint32_t val = 0, len=0;
			bool flag = false;
			for (const auto& e : h.codes_table_) {
				if (e.sym_ == mdiff(r, c)) {
					val = e.val_;
					len = e.len_;
					flag = true;
					break;
				}
			}
			if (flag)
				bw.write(val, len);
			else {
				cout << "ERROR " << " " << mdiff(r, c) << endl;
			}

		}
	}
	return true;

}

bool c_huffdiff(std::string input_f, std::string out_f) {

	mat<uint8_t> m;
	load_pam(m, input_f);
	mat<int> mdiff = calc_diffm(m);

	// mat<uint8_t> viewable_mdiff = calc_viewable_diffm(mdiff);
	
	// save_pam(viewable_mdiff, "diff.pam");

	huffman<int> h;
	std::unordered_map<int, uint32_t> freq;

	for (size_t r = 0; r < mdiff.rows(); r++) {
		for (size_t c = 0; c < mdiff.cols(); c++) {
			int f = mdiff(r, c);
			freq[f]++;
		}
	}

	h.compute_codes_table(freq);
	h.compute_canonical_codes();
	// h.print_canonical_codes();
	generate_huffdiff(mdiff, out_f, h);



	return true;
}

void error(std::string) {
	return;
}


void g_decode(const std::string& input, const std::string& output)
{
	using namespace std;
	ifstream is(input, ios::binary);
	if (!is) {
		error("Cannot open input file\n");
	}

	string MagicNumber(8, ' ');
	raw_read(is, MagicNumber[0], 8);
	if (MagicNumber != "HUFFDIFF") {
		error("Wrong input format\n");
	}

	int Width, Height;
	raw_read(is, Width);
	raw_read(is, Height);

	bitreader br(is);
	uint32_t tmp;
	tmp=br.read(9);
	size_t TableEntries = tmp;

	huffman<int> h;
	for (size_t i = 0; i < TableEntries; ++i) {
		huffman<int>::code t;
		t.sym_=br.read(9);
		t.len_=br.read( 5);
		h.codes_table_.push_back(t);
	}
	h.compute_canonical_codes();

	mat<int> diff(Height, Width);

	for (int r = 0; r < diff.rows(); ++r) {
		for (int c = 0; c < diff.cols(); ++c) {
			uint32_t len = 0, code = 0;
			size_t pos = 0;
			do {
				while (h.codes_table_[pos].len_ > len) {
					uint32_t bit;
					bit=br.read(1);
					code = (code << 1) | bit;
					++len;
				}
				if (code == h.codes_table_[pos].val_) {
					break;
				}
				++pos;
			} while (pos < h.codes_table_.size());
			if (pos == h.codes_table_.size()) {
				error("This shouldn't happen!\n");
			}
			diff(r, c) = h.codes_table_[pos].sym_;
		}
	}

	mat<uint8_t> img(diff.rows(), diff.cols());
	int prev = 0;
	for (int r = 0; r < diff.rows(); ++r) {
		for (int c = 0; c < diff.cols(); ++c) {
			img(r, c) = diff(r, c) + prev;
			prev = img(r, c);
		}
		prev = img(r, 0);
	}

	save_pam(img, output);
}

bool d_huffdiff(std::string input_f, std::string out_f) {

	using namespace std;
	ifstream in(input_f, ios::binary);
	bitreader br(in);

	huffman<int> h;
	string mn(8, ' ');

	//in.read(&mn[0], 8);
	raw_read(in, mn[0], 8);
	if (mn != "HUFFDIFF")
		return false;

	uint32_t H = 0, W = 0, numElement=0;
	raw_read(in, H);
	raw_read(in, W);

	mat<int> mdiff(H,W);

	numElement = br.read(9);

	// prendo i valori di simbolo e lunghezza (2 su 3)
	for (size_t i = 0; i < numElement; i++) {
		huffman<int>::code c;

		c.sym_ = br.read(9);
		c.len_ = br.read(5);

		h.codes_table_.push_back(c);
	}

	// calcolo il 3 mancante, ovvero val_
		// ricalcolo quindi i codici canonici
	h.compute_canonical_codes();

	//h.print_canonical_codes();

	cout << endl << "read size: " << h.codes_table_.size() << endl;

	for (int r = 0; r < mdiff.rows(); ++r) {
		for (int c = 0; c < mdiff.cols(); ++c) {
			uint32_t len = 0, code = 0;
			size_t pos = 0;
			do {
				// è effettiavmente meglio di una ricerca iterativa? sì
				// matching sulla lunghezza su tutta la codes_table_ 

				while (h.codes_table_[pos].len_ > len) {
					uint32_t bit;
					bit = br.read(1);
					code = (code <<= 1) | bit;
					len++;
				}
				if (code == h.codes_table_[pos].val_) {
					break;
				}
				++pos;
			} while (pos < h.codes_table_.size());
			if (pos == h.codes_table_.size()) {
				error("This shouldn't happen!\n");
			}
			mdiff(r, c) = h.codes_table_[pos].sym_;
		}
		
	}



	mat<uint8_t> img(H, W);
	int prev = 0;
	for (int r = 0; r < mdiff.rows(); ++r) {
		for (int c = 0; c < mdiff.cols(); ++c) {
			img(r, c) = mdiff(r, c) + prev;
			prev = img(r, c);
		}
		prev = img(r, 0);
	}

	save_pam(img, out_f);
	
	return true;
}


int main(int argc, char** argv)
{
	if (argc != 4)
		return -1;

	switch (*argv[1]) {
	case 'c': {
		c_huffdiff(argv[2], argv[3]);
		break;
	}
	case'd': {
		g_decode(argv[2], argv[3]);
		break;
	}
	default:
		return -1;
	}


	return EXIT_SUCCESS;
}
