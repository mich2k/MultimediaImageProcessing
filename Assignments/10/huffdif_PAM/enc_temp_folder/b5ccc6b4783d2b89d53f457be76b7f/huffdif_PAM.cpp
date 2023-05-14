// huffdif_PAM.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>
#include <fstream>
#include <cassert>

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

	mat<int> mdiff;
	mdiff.resize(m.rows(), m.cols());

	for (size_t r = 0; r < m.rows(); ++r) {
		for (size_t c = 0; c < m.cols(); ++c) {

			if (r == 0 && c == 0) {
				mdiff(r, c) = m(r, c);
				continue;
			}
			if (r == 0 && c > 0) {
				mdiff(r, c) = m(0, c) - m(0, c - 1);
				continue;
			}
			if (r > 0) {
				mdiff(r, c) = m(r, c) - m(r - 1, c);
			}

		}
	}

	return mdiff;
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

bool generate_huffdiff(mat<int> m, std::string out_filename) {

	using namespace std;
	ofstream os(out_filename, ios::binary);
	
	os << "HUFFDIFF";
	os << static_cast<uint32_t>(m.cols());
	os << static_cast<uint32_t>(m.rows());

	return true;

}

bool c_huffdiff(std::string input_f, std::string out_f) {

	mat<uint8_t> m;
	load_pam(m, input_f);
	mat<int> mdiff = calc_diffm(m);

	mat<uint8_t> viewable_mdiff = calc_viewable_diffm(mdiff);
	
	save_pam(viewable_mdiff, "diff.pam");



	return true;
}

void d_huffdiff(std::string input_f, std::string out_f) {
	return;
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
		break;
	}
	default:
		return -1;
	}


	return EXIT_SUCCESS;
}
