#include "pgm16.h"
using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	in.read(reinterpret_cast<char*>(&v), sizeof(T));
	return in;
}


bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {
	string mw;
	ifstream in(filename, ios::binary);

	in >> mw;
	string t;
	char c;
	int W, H;

	if (mw != "P5")
		return false;

	in.get(c);
	if (c != '\n')
		return false;

	if (in.peek() == '#') {
		string comment;
		getline(in, comment);
	}

	in >> t;
	W = stoi(t);
	in.get(c);
	if (c != ' ')
		return false;
	in >> t;
	in.get(c);
	if (c != '\n')
		return false;
	H = stoi(t);

	img.resize(H, W);

	in >> t;
	maxvalue = stoi(t);

	in.get(c);
	if (c != '\n')
		return false;

	if (maxvalue < 256) {
		for (int r = 0; r < img.rows(); ++r) {
			for (int c = 0; c < img.cols(); ++c) {
				if (r == 32 && c == 228)
					cout << "x";
				uint8_t x;
				raw_read(in, x);	// l'estrattore ignora i whitespace, non va bene
				img(r, c) = x;

			}
		}
	}
	else {
		for (int r = 0; r < img.rows(); ++r) {
			for (int c = 0; c < img.cols(); ++c) {
				uint16_t val = 0;
				uint8_t x;
				raw_read(in, x);
				val |= x;
				val <<= 8;
				raw_read(in, x);
				val |= x;
				img(r, c) = val;

			}
		}

	}

	return true;
}



//int main(int argc, char** argv)
//{
//    mat<uint16_t> m;
//    uint16_t maxvalue=0;
//    string filename = "frog_bin.pgm";
//	load(filename, m, maxvalue);
//
//    return EXIT_SUCCESS;
//}


/*
I want to load file "frog_bin.pgm".
Calling load()... done.
Error: load() set img._data wrong. The first difference is at img(32,229), which has value 4, but it should have been 10.
I want to load file "CR-MONO1-10-chest.pgm".
Calling load()... done.
Error: load() set img._data wrong. The first difference is at img(0,20), which has value 771, but it should have been 779.
I want to load file "non_existing_file".
Calling load()... done.
Ok: load() returned false.


*/




/*

I want to load file "frog_bin.pgm".
Calling load()... done.
Error: load() set img._data wrong. The first difference is at img(0,0), which has value 20817, but it should have been 81.
I want to load file "CR-MONO1-10-chest.pgm".
Calling load()... done.
Error: load() set img._data wrong. The first difference is at img(0,1), which has value 53504, but it should have been 209.
I want to load file "non_existing_file".
Calling load()... done.
Ok: load() returned false.

*/