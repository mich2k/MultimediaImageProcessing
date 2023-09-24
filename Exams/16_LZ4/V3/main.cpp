
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>

using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T>
ostream& raw_write(ostream& os, const T& v) {
	return os.write(reinterpret_cast<const char*>(&v), sizeof(T));
}


class lz4 {
private:
	istream& _in;
	ostream& _os;
	vector<uint8_t> _decoded;
	uint32_t _dec_len;


	void read_blocks() {
		uint32_t block_len = 0;

		while (raw_read(_in, block_len)) {
			uint32_t bytes = 0;

			while (true) {
				if (_in.fail() || _in.eof())
					return;

				uint8_t token = 0;
				uint8_t lit_len = 0, match_len = 0;

				raw_read(_in, token);

				lit_len |= (token >> 4);
				match_len |= (token & 0b0000'1111);
				bytes++;

				if (lit_len == 15) {
					uint8_t byte = 0;
					do {
						raw_read(_in, byte);
						lit_len += byte;
						bytes++;
					} while (byte == 255);
				}

				vector<uint8_t> literals;
				for (size_t i = 0; i < lit_len; ++i) {
					uint8_t c;
					raw_read(_in, c);
					literals.push_back(c);
					_decoded.push_back(c);
					bytes++;
				}
				//for (auto l : literals) {
				//	_decoded.push_back(l);
				//}


				if (bytes == block_len)
					break;

				uint16_t offset = 0;
				raw_read(_in, offset);
				bytes += 2;

				if (match_len == 15) {
					uint8_t byte = 0;
					do {
						raw_read(_in, byte);
						match_len += byte;
						bytes++;
					} while (byte == 255);
				}
				match_len += 4;

				vector<uint8_t> run;

				int starting_point = _decoded.size() - offset;
				while (match_len > offset) {
					for (int index = starting_point; index < _decoded.size(); ++index) {
						run.push_back(_decoded[index]);
					}
					match_len -= offset;
				}
				for (int index = starting_point; index < starting_point + match_len; ++index) {
					run.push_back(_decoded[index]);
				}


				//for (int index = 0; index < match_len; ++index) {
				//	int in = starting_point + (index % offset);
				//	if (in > _decoded.size())
				//		in = starting_point;
				//	char c = _decoded[in];
				//	run.push_back(_decoded[in]);
				//}

				for (auto e : run)
					_decoded.push_back(e);

			}

		}
	}


public:
	lz4(ifstream& in, ofstream& os) : _in(in), _os(os), _dec_len(0) {};



	bool check_header() {
		uint32_t x = 0;
		raw_read(_in, x);

		if (x != 0x184c2103)
			return false;

		raw_read(_in, x);
		_dec_len = x;

		raw_read(_in, x);
		if (x != 0x4d000000)
			return false;


		return true;
	}

	bool decode() {


		read_blocks();

		for (auto e : _decoded)
			raw_write(_os, e);

		return true;
	}


};



int main(int argc, char** argv) {

	ifstream in(argv[1], ios::binary);
	ofstream os(argv[2], ios::binary);

	lz4 l(in, os);

	if (!l.check_header())
		return EXIT_FAILURE;

	l.decode();

	return EXIT_SUCCESS;
}