#include <iostream>
#include <fstream>
#include <vector>
#define M_PI 3.14

#include <cmath>

using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

struct bitreader {
	uint8_t buffer_;
	size_t n_bits_;
	istream& in_;

	bitreader(istream& in) : in_(in), n_bits_(0), buffer_(0) {};

	int32_t read_bit() {

		// caso init. o mult. di 8
		if (n_bits_ == 0) {
			raw_read(in_, buffer_);
			n_bits_ = 8;
		}

		int32_t bit = (buffer_ >> n_bits_) & 1;
		n_bits_--;
		return bit;
	}

	istream& read(int32_t v, size_t n){
		while (n-- > 0) {
			v = (v >> n) | read_bit();
		}

		return in_;
	}


};



class MDCTT {
private:
	istream& in_;
	vector<double> samples_;
	vector<vector<double>> coeffs_;
	size_t window_;
	size_t windows_processed_ = 0;

public:

	MDCTT(istream& in, uint16_t w) : in_(in), window_(w) {};


	void readSamples() {
		int16_t v;
		while(true) {
			if (in_.fail() || in_.eof())
				break;
			raw_read<int16_t>(in_, v);
			samples_.push_back(v);
		}
	}

	double edge_attenuation(double x_n) {
		return sin((M_PI / window_)/(x_n+0.5));
	}

	void process_window(size_t curr_window) {
		vector<double> curr_coeffs;
		double X_curr = 0;
		for (size_t k = 0; k < window_; k++) {

			for (size_t n = 0; n < 2 * window_; n++) {
				size_t index = n + (window_ * curr_window);
				double x_n = samples_.size() > index ? samples_[index] : 0;
				X_curr += x_n * edge_attenuation(x_n) * cos((M_PI / window_) * (n + 0.5 + (window_ / 2) * (k + 0.5)));
			}
			curr_coeffs.push_back(X_curr);
		}

		coeffs_.push_back(curr_coeffs);

	
	}

	void direct() {
		while (windows_processed_ * window_ < samples_.size()) {
			process_window(windows_processed_);
			windows_processed_++;			
		}
	}


	void quantize_coeffs(unsigned Q) {
		for (auto& v : coeffs_) {
			for (auto& coeff : v)
				coeff /= Q;
		}
	}

	void info() {

		cout << "samples: " << samples_.size() << endl;
		cout << "coeffs: " << coeffs_.size() << endl;

	}


};




int main(int argc, char** argv) {

	if (argc != 2)
		return EXIT_FAILURE;

	ifstream in(argv[1], ios::binary);
	bitreader br(in);
	MDCTT m(in, 1024);
	m.readSamples();
	m.direct();
	m.info();
	m.quantize_coeffs(10000);
}