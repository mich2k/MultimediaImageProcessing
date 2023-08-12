#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	in.read(reinterpret_cast<char*>(&v), sizeof(T));
	return in;
}


class LZ4 {
private:
	istream& _in;
	bool _legitStream = false;
	uint32_t _len;
	string _out_filename = "";
	bool _finalBlockReached = false;
	vector<uint8_t> _literals;
public:


	LZ4(ifstream& in, string out_fn) : _in(in), _len(0), _out_filename(out_fn) {};


	void readHeader() {

		//uint8_t mw[4];
		//for (uint8_t i = 0; i < 4; i++)
		//	_in >> mw[i];

		//_legitStream = mw[0] != 0x03 ? false : true;
		//_legitStream = mw[1] != 0x21 ? false : true;
		//_legitStream = mw[2] != 0x4c ? false : true;
		//_legitStream = mw[3] != 0x18 ? false : true;

		// uint32_t wholeMw = mw[0] + (mw[1] << 8) + (mw[2] << 16) + (mw[3] << 24);


		uint32_t  mw = 0;
		raw_read(_in, mw);
		if (mw != 407642371)
			return;


		uint8_t len[4];
		for (uint8_t i = 0; i < 4; i++)
			_in >> len[i];

		_len = len[0] + (len[1] << 8) + (len[2] << 16) + (len[3] << 24);

		uint8_t cv = 255;

		_in >> cv;
		if (cv != 0x00)
			return;
		_in >> cv;
		if (cv != 0x00)
			return;
		_in >> cv;
		if (cv != 0x00)
			return;
		_in >> cv;
		if (cv != 0x4d)
			return;

		_legitStream = true;
	}


	void readBlock() {


		// block len (un ora per capire che questo valore esiste, assente su 2 format spec. su 3, ok)

		uint32_t block_len = 0;
		raw_read(_in, block_len);

		if (_in.fail() || _in.eof()) {
			_finalBlockReached = true;
			return;
		}


		uint32_t bytes_read = 0;

		while (bytes_read <= block_len) {

			// token

			uint8_t token;
			raw_read(_in, token);
			bytes_read += 1;


			uint32_t literals_len = token >> 4;
			uint8_t match_len = token & 0b0000'1111;


			// unsuccessful since 0 means 4 
			// hence having ml == 0 doesnt mean block is over
			// 
			// Block _literals only condition
			//if (match_len == 0) {
			//	for (uint8_t i = 0; i < literals_len; i++) {
			//		uint8_t c = 0;
			//		raw_read(_in, c);
			//		bytes_read += 1;
			//		cout << c;
			//	}
			//	break;
			//}


			// optional literal length bytes

			if (literals_len == 15) {
				while (true) {
					uint8_t byte = 0;
					raw_read(_in, byte);
					literals_len += byte;
					bytes_read += 1;
					if (byte != 255)
						break;
				}
			}

			// _literals
			for (uint32_t i = 0; i < literals_len; i++) {
				uint8_t c;
				raw_read(_in, c);
				_literals.push_back(c);
				// the problem "statement" also misses to "state
				// that literals are part of the decoded, ok
				cout << c;
				bytes_read += 1;

			}


			// literals only at end of block
			if (bytes_read == block_len)
				break;


			//for (auto e : _literals)
			//	cout << e;

			uint16_t offset = 0, t = 0;
			raw_read(_in, offset);
			bytes_read += 2;

			// fixing endianess
			//offset |= (t << 8);
			//offset |= (t >> 8);

			// reading optional match length

			match_len += 4; // adding minmatch

			if (match_len == 19) {
				while (true) {
					uint8_t byte = 0;
					raw_read(_in, byte);
					match_len += byte;
					bytes_read += 1;
					if (byte != 255)
						break;
				}
			}

			vector<uint8_t> repSegment(_literals.end() - (offset) , _literals.end());

			for (size_t i = 0; i < match_len; i++) {
				uint8_t c = repSegment[i % offset];
				_literals.push_back(c);
				cout << c;
			}
		}

	}


	bool checkIfLessThanTwelve() {
		int pos = _in.tellg();
		_in.seekg(0,_in.end);
		int end = _in.tellg();
		int len = end - pos;
		_in.seekg(pos);
		return len <= 12;
	}

	void decode() {

		if (checkIfLessThanTwelve()) {
			while (true) {
				uint8_t c = 0;
				raw_read(_in, c);
				if (_in.fail() || _in.eof())
					break;
				cout << c;
			}
			return;
		}

		while (!_finalBlockReached) {
			readBlock();
			_literals.clear();
		}
	}


	bool isCompatibleFilestream() {
		return _legitStream;
	}


};



int main(int argc, char** argv) {
	if (argc != 3)
		return EXIT_FAILURE;

	ifstream in(argv[1], ios::binary);
	LZ4 lz4(in, argv[2]);
	lz4.readHeader();
	if (!lz4.isCompatibleFilestream())
		return EXIT_FAILURE;
	lz4.decode();


	return EXIT_SUCCESS;
}