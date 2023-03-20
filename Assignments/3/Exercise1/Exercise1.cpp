// Exercise1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <cstdint>
#include <iostream>
#include <iterator>
#include <vector>
#include <fstream>
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

/*
class elias {
	std::vector<int> v_compression_;
	std::ofstream& out_;
	bool mode_;

public:
	elias(bool mode, std::ifstream& fs, const char* filename) {
		mode_ = mode;
		if (mode == 'c') {
			fs = std::fopen(filename, "r");
		}
		else {
			if (mode == 'd') {
				// pass
			}
		}
	}


	void map() {
		
	}



};*/


int map(std::vector<int>& v_mapped, std::ifstream& in) {
	
	std::istream_iterator<int32_t> in_start(in);
	std::istream_iterator<int32_t> in_stop;
	std::vector<int> v = { in_start, in_stop };

	// v.assign(in_start, in_stop);

	for (auto it = v.begin(); it != v.end(); it++) {
		std::cout << *it << std::endl;
	}

	for (auto& elem : v) {
		if (elem == 0) {
			v_mapped.push_back(0);
			continue;
		}
		if (elem > 0) {
			v_mapped.push_back(elem * 2 + 1);
			continue;
		}

		int t = elem;
		t *= -2;
		v_mapped.push_back(t);

	}
	return 0;
}



int main(int argc, char* argv[])
{
	using namespace std;

	if (argc != 4) {
		cout << "Usage: elias [c|d] <filein> <fileout>" << endl;
		return(EXIT_FAILURE);
	}

	ifstream in;
	ofstream out;

	char mode = *argv[1];

	switch (mode) {
	case 'c': {
		in.open(argv[2], ifstream::in);
		out.open(argv[3], ios::binary);
		break;
	}
	case 'd': {
		in.open(argv[2], ios::binary);
		out.open(argv[3]);
		break;
	}
	default:
		cout << "Usage: elias [c|d]" << endl;
		return EXIT_FAILURE;
		break;
	}

	if (in.fail() || out.fail()) {
		cout << "File open failure" << endl;
		return EXIT_FAILURE;
	}

	switch (mode) {
	case 'c': {
		//vector<int> v_mapped;


		std::istream_iterator<int32_t> in_start(in);
		std::istream_iterator<int32_t> in_stop;
		std::vector<int32_t> v_mapped = { in_start, in_stop };

		cout << "Size: " << v_mapped.size() << endl;
		for (int i = 0; i < v_mapped.size(); i++) {
			cout << v_mapped[i] << endl;
		}

		//map(v_mapped, in);

		for (int e : v_mapped) {
			cout << e << endl;
		}
		break;


	}
	}





	return EXIT_SUCCESS;
}
