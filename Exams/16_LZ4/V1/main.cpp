#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T>
ostream& raw_write(ostream& os, const T& v) {
	return os.write(reinterpret_cast<const char*>(&v), sizeof(T));
}


class LZ4 {

private:
	istream& _in;
	ostream& _os;
	bool _legitStream = false;
	uint32_t _len;
	string _out_filename = "";
	bool _finalBlockReached = false;
	vector<uint8_t> _literals;
	vector<uint8_t> _seq;



	bool checkIfLessThanTwelve() {
		int pos = _in.tellg();
		_in.seekg(0, _in.end);
		int end = _in.tellg();
		int len = end - pos;
		_in.seekg(pos);
		return len <= 12;
	}




	void start_decode() {

		while (!_finalBlockReached) {
			readBlock();
			//_literals.clear();
		}
	}


	void insert_sequence(std::vector<uint8_t>& seq) {
		for (auto& byte : seq) {
			_seq.push_back(byte);
		}
	}

	vector<uint8_t> get_run(size_t offset, size_t match_length) {
		vector<uint8_t> run;
		int starting_point = _seq.size() - offset;

		for (int index = 0; index < match_length; ++index) {
			int in = starting_point + (index % offset);
			run.push_back(_seq[in]);
		}

		return run;
	}


	void readBlock() {


		// block len (un ora per capire che questo valore esiste, assente su 2 format spec. su 3, ok)

		uint32_t block_len = 0;

		if (!raw_read(_in, block_len)) {
			_finalBlockReached = true;
			return;
		}
		uint32_t bytes_read = 0;


		// token

		uint8_t token;
		while (raw_read(_in, token)) {
			bytes_read += 1;


			uint32_t literals_len = (token >> 4);
			uint8_t match_len = token & 0b0000'1111;

			// optional literal length bytes

			if (literals_len == 15) {
				uint8_t byte = 0;
				while (raw_read(_in, byte)) {
					literals_len += byte;
					bytes_read += 1;
					if (byte != 255)
						break;
				}
			}

			// _literals
			_literals.clear();
			for (uint32_t i = 0; i < literals_len; i++) {
				uint8_t c;
				raw_read(_in, c);
				_literals.push_back(c);
				// the problem "statement" also misses to state
				// that literals are part of the decoded, ok

				raw_write(_os, c);
				//cout << c;

			}
			bytes_read += literals_len;

			insert_sequence(_literals);


			// literals only at end of block
			if (bytes_read == block_len)
				break;


			uint16_t offset = 0, t = 0;
			raw_read(_in, offset);
			bytes_read += 2;

			if (offset == 0 || offset == 65536) {
				cout << "ERROR" << endl;
				exit(1);
			}

			// reading optional match length


			if (match_len == 15) {
				uint8_t byte = 0;
				while (raw_read(_in, byte)) {
					match_len += byte;
					bytes_read += 1;
					if (byte != 255)
						break;
				}
			}
			match_len += 4; // adding minmatch


			vector<uint8_t> run = get_run(offset, match_len);
			insert_sequence(run);
			for (auto& e : run)
				raw_write(_os, e);

		}
	}

public:


	LZ4(ifstream& in, ofstream& out) : _in(in), _len(0), _os(out) {};

	void readHeader() {
		uint32_t  mw = 0;
		raw_read(_in, mw);
		if (mw != 407642371)
			return;

		uint32_t len = 0;
		raw_read(_in, len);

		uint32_t cv = 0;
		raw_read(_in, cv);
		if (cv != 0x4D000000)
			return;

		_legitStream = true;
	}


	void decode() {
		if (checkIfLessThanTwelve()) {
			uint8_t c = 0;
			while (raw_read(_in, c)) {
				if (_in.fail() || _in.eof())
					break;
				raw_write(_os, c);
			}
			return;
		}

		start_decode();
	}



	bool isCompatibleFilestream() {
		return _legitStream;
	}


};



int main(int argc, char** argv) {
	if (argc != 3)
		return EXIT_FAILURE;

	ifstream in(argv[1], ios::binary);
	ofstream out(argv[2], ios::binary);
	LZ4 lz4(in, out);
	lz4.readHeader();
	if (!lz4.isCompatibleFilestream())
		return EXIT_FAILURE;
	lz4.decode();


	return EXIT_SUCCESS;
}