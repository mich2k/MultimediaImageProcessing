#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <array>

using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T>
ostream& raw_write(ostream& out, const T& v) {
	return out.write(reinterpret_cast<const char*>(&v), sizeof(T));
}


using pixel = array<uint8_t, 3>;

class pam {
private:
	int _width, _height, _maxval;
	vector<vector<pixel>>& _data;
	ostream& _out;


	void write_header() {
		_out << "P6\n";
		_out << _width << "\n";
		_out << _height << "\n";
		_out << _maxval << "\n";

	}

public:
	
	pam(int width, int height, int maxval, ostream& out, vector<vector<pixel>>& data) : _width(width), _height(height), _maxval(maxval), _out(out), _data(data){
		this->write_header();
	};


	
	void writeData() {

		for (int r = 0; r < _data.size(); r++) {
			for (int c = 0; c < _data[0].size(); c++) {
				raw_write(_out, _data[r][c]);

			}
		}

	}

	void writePixelData() {

		for (int r = 0; r < _data.size(); r++) {
			for (int c = 0; c < _data[0].size(); c++) {
				raw_write(_out, _data[r][c][0]);
				raw_write(_out, _data[r][c][1]);
				raw_write(_out, _data[r][c][2]);


			}
		}


		//for (auto& row : _data) {
		//	for (auto& e : row) {
		//		raw_write(_out, e[0]);
		//		raw_write(_out, e[1]);
		//		raw_write(_out, e[2]);

		//	}
		//}
	}

	auto getRows (){
		return this->_height;
	}

	auto getCols() {
		return this->_width;
	}

	auto getData() {
		return this->_data;
	}

};


class pgm {
private:
	istream& _in;
	ostream& _out;
	uint32_t _width;
	uint32_t _height;
	uint16_t _maxval;
	vector<vector<uint16_t>> _data;
	vector<vector<pixel>> _labeledData;

	bool readHeader() {
		string mw = "  ";
		_in.read(&mw[0], 2);
		if (mw != "P5")
			return false;
		_in.ignore();
		_in >> _width;
		_in.ignore();
		_in >> _height;
		_in.ignore();
		_in >> _maxval;
		_in.ignore();

		return true;
	}

	void readData() {
		vector<uint16_t> row;

		for (size_t r = 0; r < _height; r++) {
			for (size_t c = 0; c < _width; c++) {
				uint16_t raw16 = 0;

				raw_read(_in, raw16);

				uint16_t first_byte = (raw16 & 0b1111'1111'0000'0000);
				uint16_t second_byte = (raw16 & 0b0000'0000'1111'1111);

				first_byte >>= 8;
				second_byte <<= 8;

				uint16_t corrected_byte = first_byte | second_byte;
				row.push_back(corrected_byte);

			}
			_data.push_back(row);
			row.clear();
		}

	}




public:
	pgm(istream& in, ostream& out) : _in(in), _out(out) {};

	auto getByerLabeledData() {
		return this->_labeledData;
	}


	auto getW() {
		return this->_width;
	}
	auto getH() {
		return this->_height;
	}


	bool go() {
		if (!readHeader())
			return false;
		readData();


		return true;
	}

	void to8bpp() {
		for (auto& row : _data) {
			for (auto& e : row) {
				e /= 256;
			}
		}
	}


	void save() {

		_out << "P5\n";
		_out << _width;
		_out << "\n";
		_out << _height;
		_out << "\n";
		_out << "255\n";

		for (auto& row : _data) {
			for (auto& e : row) {
				uint8_t b = e;
				raw_write(_out, b);	// needed or the raw_write would pick as sizeof(T) 16 bites, no jokin
			}
		}

	}

	void labelColor(vector<vector<uint8_t>>& scheme) {
		/*
			Bayer scheme:
			RGRGRGRGRGRGRG..	0
			GBGBGBGBGBGBGB..	1
		*/

		// so elegant, this ocd is my ruin, next exam everything inside int main() {}

		bool rowIdentity = 0;	// RGRG or GBGB
		bool columnIdentity = 0;	// R or G // G or B
		int r=0, c=0;
		for (auto& row : _data) {
			vector<pixel> labeledRow;
			vector<uint8_t> schemeRow;
			pixel p = { 0,0,0 };

			for (auto e : row) {
				uint8_t v = e;	// :(

				switch (rowIdentity) {
					break;case 0:
						if (columnIdentity == 0) {
							p = { v,0,0 };
							schemeRow.push_back('r');
						}
						else {
							p = { 0,v,0 };
							schemeRow.push_back('g');

						}
						columnIdentity = !columnIdentity;

					break;case 1:
						if (columnIdentity == 0) {
							p = { 0,v,0 };
							schemeRow.push_back('g');

						}
						else {
							p = { 0,0,v };
							schemeRow.push_back('b');
						}
						columnIdentity = !columnIdentity;

					break; default:
						cout << "wait, is a boolean!" << endl;
				}

				labeledRow.push_back(p);
				c++;

			}
			rowIdentity = !rowIdentity;
			_labeledData.push_back(labeledRow);
			scheme.push_back(schemeRow);

			r++;
		}


	}


};


auto interpolate_green(pam& m, vector<vector<uint8_t>>& scheme) {
	
	vector<vector<pixel>> d = m.getData();

	for (int r = 0; r < m.getRows(); r++) {
		for (int c = 0; c < m.getCols(); c++) {


			// green pixel continue
			if (scheme[r][c] == 'g')
				continue;

			uint8_t X5_color_array_index = scheme[r][c] == 'r' ? 0 : scheme[r][c] == 'g' ? 1 : 2;

			uint8_t X5, X3, X7, X1, X9;
			uint8_t G2, G4, G6, G8;

			// is X5 a blue or a red?


			try {
				X5 = d.at(r).at(c)[X5_color_array_index];
			} catch (out_of_range const& exc) {
				//cout << exc.what() << " X5";
			}

			try {
				G4 = d.at(r).at(c-1)[0];
			} catch (out_of_range const& exc) {
				G4 = 0;
				//cout << exc.what() << " G4";
			}			
			
			try {
				X3 = d.at(r).at(c-2)[X5_color_array_index];
			} catch (out_of_range const& exc) {
				X3 = 0;
				//cout << exc.what() << " X3";
			}			
			
			try {
				G6 = d.at(r).at(c)[0];
			} catch (out_of_range const& exc) {
				G6 = 0;
				//cout << exc.what() << " G6";
			}			
			
			try {
				X7 = d.at(r).at(c)[X5_color_array_index];
			} catch (out_of_range const& exc) {
				X7 = 0;
				//cout << exc.what() << " X7";
			}			
			
			try {
				G2 = d.at(r-1).at(c)[0];
			} catch (out_of_range const& exc) {
				G2 = 0;
				//cout << exc.what() << " X5";
			}		
			
			try {
				X1 = d.at(r-2).at(c)[X5_color_array_index];
			} catch (out_of_range const& exc) {
				X1 = 0;
				//cout << exc.what() << " X1";
			}			
			
			try {
				G8 = d.at(r+1).at(c)[0];
			} catch (out_of_range const& exc) {
				G8 = 0;
				//cout << exc.what() << " G8";
			}			
			
			try {
				X9 = d.at(r+2).at(c)[X5_color_array_index];
			} catch (out_of_range const& exc) {
				X9 = 0;
				//cout << exc.what() << " X9";
			}


			int de_h = abs(G4 - G6) + abs(2 * X5 - X3 - X7);
			int de_v = abs(G2 - G8) + abs(2 * X5 - X1 - X9);

			uint8_t G5 = 0;

			// odiatemi

			G5 = de_h < de_v ? ((G4 + G6) / 2 + (X5 - X3 - X5 - X7) / 4) : de_h > de_v ? ((G2 + G8) / 2 + (X5 - X1 - X5 - X9) / 4) : ((G2 + G4 + G6 + G8) / 4 + (X5 - X1 + X5 - X3 + X5 - X7 + X5 - X9) / 8);


			d[r][c][1] = G5;

		}


	}

	return d;

}


auto interpolate_second(pam& m, vector<vector<uint8_t>>& scheme) {
	vector<vector<pixel>> d = m.getData();

	for (int r = 0; r < m.getRows(); r++) {
		for (int c = 0; c < m.getCols(); c++) {

			if (scheme[r][c] == 'g') {
				uint8_t vertical = 0;

				// redundant check if vertical is blue
				try {
					if (scheme.at(r - 1).at(c) == 'b') {
						vertical = 2;
					}
				} catch (const std::out_of_range& oor) {
					if (scheme.at(r + 1).at(c) == 'b') {
						vertical = 2;
					}
				}

				// redundant check if vertical is red

				try {
					if (scheme.at(r - 1).at(c) == 'r') {
						vertical = 0;
					}
				} catch (const std::out_of_range& oor) {
					if (scheme.at(r + 1).at(c) == 'r') {
						vertical = 0;
					}
				}

				// interpolate

				uint8_t up, down;
				try {
					up = d.at(r - 1).at(c)[vertical];
					try {
						// case R1 ON, R2 ON
						down = d.at(r + 1).at(c)[vertical];

					} catch (const std::out_of_range& oor) {
						// case R1 ON, R2 OFF
						down = 0;

					}


				} catch (const std::out_of_range& oor) { // possibly useless condition
					// case R1 OFF, R2 ON
					up = 0;
					try {
						down = d.at(r + 1).at(c)[vertical];
					} catch (const std::out_of_range& oor) {
						// R1 OFF, R2 OFF
						down = 0;				// useless too

					}
				};

				d[r][c][vertical] = (up + down) / 2;

				uint8_t horiz = vertical == 2 ? 0 : 2;
				uint8_t left, right;
				try {
					left = d.at(r).at(c - 1)[horiz];

					try {
						right = d.at(r).at(c + 1)[horiz];

					} catch (const std::out_of_range& oor) {
						right = 0;

					}
				} catch (const std::out_of_range& oor) {
					left = 0;

					try {
						right = d.at(r).at(c + 1)[horiz];

					} catch (const std::out_of_range& oor) {
						right = 0;

					}
				}

				d[r][c][horiz] = (left + right) / 2;


			} else {

				// pixel R & B

				int16_t X1, X3, X7, X9, G1=0, G3=0, G5=0, G7=0, G9=0;

				uint8_t source = scheme[r][c] == 'r' ? 2 : 0;
				uint8_t dest = scheme[r][c] == 'r' ? 0 : 2;
				// B Interpolation
				try {
					X1 = d.at(r -1).at(c -1)[source];
					G1 = d.at(r - 1).at(c - 1)[1];
				} catch (out_of_range const& exc) {
					X1 = 0;
				}
				try {
					X3 = d.at(r - 1).at(c + 1)[source];
					G3 = d.at(r - 1).at(c + 1)[1];

				} catch (out_of_range const& exc) {
					X3 = 0;
				}
				try {
					X7 = d.at(r - 1).at(c - 1)[source];
					G7 = d.at(r - 1).at(c - 1)[1];

				} catch (out_of_range const& exc) {
					X7 = 0;
				}
				try {
					X9 = d.at(r + 1).at(c + 1)[source];
					G9 = d.at(r + 1).at(c + 1)[1];

				} catch (out_of_range const& exc) {
					X9 = 0;
				}



				int16_t de_n, de_p;


				de_n = abs(X1 - X9) + abs(G5 - G1 + G5 - G9);
				de_p = abs(X3 - X7) + abs(G5 - G3 + G5 - G7);

				int16_t v = de_n < de_p ? ((X1 - X9) / 2 + (G5 - G1 + G5 - G9) / 4) : de_n > de_p ? ((X3 + X7) / 2 + (G5 - G3 + G5 - G7) / 4) : ((X1 + X3 + X7 + X9) / 4 + (G5 - G1 + G5 - G3 + G5 - G7 + G5 - G9) / 8);
				if (v > 0) {
					cout << "a";
				}
				v = v > 255 ? 255 : v;
				v = v < 0 ? 0 : v;

				uint8_t v8 = v;

				d[r][c][dest] = v8;



			};




		}



	}
	return d;

}

int main(int argc, char** argv) {

	if (argc != 3) {
		cout << "bayer_decode <input file.PGM> <output prefix>" << endl;
		return 1;
	}

	ifstream in(argv[1], ios::binary);
	string pgm_out(argv[2]);
	pgm_out = pgm_out + "_grey.pgm";
	ofstream out(pgm_out, ios::binary);

	if (!in || !out)
		return 1;


	vector<vector<uint8_t>> color_scheme;

	pgm p(in, out);
	p.go();
	p.to8bpp();
	p.save();
	p.labelColor(color_scheme);

	ofstream pam_out("color.pam", ios::binary);
	auto d = p.getByerLabeledData();
	pam p_labeled(p.getW(), p.getH(), 255, pam_out, d);
	p_labeled.writeData();
	p.save();
	
	auto interp_green_data = interpolate_green(p_labeled, color_scheme);

	string green_out(argv[2]);
	green_out += "_green.ppm";
	ofstream pam_green_out(green_out, ios::binary);
	pam p_inter_green(p.getW(), p.getH(), 255, pam_green_out, interp_green_data);
	p_inter_green.writePixelData();


	auto interp_second_data = interpolate_second(p_inter_green, color_scheme);

	string interp_out(argv[2]);
	interp_out += "_interp.ppm";
	ofstream pam_interp_out(interp_out, ios::binary);
	if (!pam_interp_out)
		return 1;
	pam p_interp_comp(p.getW(), p.getH(), 255, pam_interp_out, interp_second_data);
	p_interp_comp.writePixelData();



	return 0;
}