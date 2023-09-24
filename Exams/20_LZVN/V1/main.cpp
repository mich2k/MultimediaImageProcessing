#include <iostream>
#include <fstream>
#include <vector>
#include <map>


using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T>
ostream& raw_write(ostream& os, const T& v) {
	return os.write(reinterpret_cast<const char*>(&v), sizeof(T));
}



class lzvn {
private:
	istream& _in;
	ostream& _os;
	uint32_t _block_size = 0, _bytes_required = 0;
	vector<uint8_t> _decoded;
	uint16_t _latest_match_distance = 0;

	bool readHeader() {
		string mw="    ";
		_in.read(&mw[0], 4);

		if (mw != "bvxn")
			return false;
		raw_read(_in, _bytes_required);
		raw_read(_in, _block_size);		// encoded size in bytes

	}


	bool checkEOS(uint8_t b) {
		bool legitEOS = true;
		if (b != 6) {
			legitEOS = false;
		} else {
			for (uint8_t j = 0; j < 7; j++) {
				if (!raw_read(_in, b)) {
					if (b != 0)
						legitEOS = false;
				}
			}
		}
		return legitEOS;
	}

	void go_decode() {

		uint8_t b;
		bool isEOSlegit = false;

		while (raw_read(_in, b) && _decoded.size() <= this->_bytes_required) {
			opcode o(b, *this);
			if (o.getType() == "eos") {
				isEOSlegit = true;
				break;
			}
			//cout << o._type << ":" << endl;
			o.action();
			//cout << endl << o._type << " " << _decoded.size() << endl;
			//printDecoded();
			//cout << endl << endl;


		}

		printToFileDecoded();

		// first byte of EOS already read here (for EOF check)

		cout << endl << "\nEOS checked (1 is ok): " << isEOSlegit << endl;


	}


	void printDecoded() {
		for (size_t i = 0; i < _decoded.size(); i++) {
			cout << _decoded[i];
		}
	}

	void printToFileDecoded() {
		for (size_t i = 0; i < _decoded.size(); i++) {
			raw_write(_os, _decoded[i]);
		}
	}



public:
	lzvn(ifstream& in, ofstream& os) : _in(in), _os(os) {};

	uint8_t readNextByte() {
		uint8_t v = 0;
		if (!raw_read(this->_in, v))
			cout << "ERROR readNextByte" << endl;

		return v;
	}

	uint16_t getLatestMatchDistance() {
		return _latest_match_distance;
	}

	void updateLatestMatchDistance(uint16_t md) {
		_latest_match_distance = md;
	}

	void appendLiteral(uint8_t l) {
		_decoded.push_back(l);
	}

	void simpleAppendNLiterals(uint8_t literal_len) {
		if (literal_len == 0)
			return;

		for (uint8_t l = 0; l < literal_len; l++) {
			uint8_t c = 0;
			if (!raw_read(_in, c))
				cout << "error simpleAppendNLiterals" << endl;
			appendLiteral(c);
		}
	}

	uint8_t literalByCircularIndex(size_t i, uint16_t match_distance, size_t start_decoded_size) {
		int offset = start_decoded_size - match_distance;
		return _decoded[offset + (i % match_distance)];
	}

	void addMatchToDecoded(uint8_t match_len, uint16_t match_distance) {
		// 1st issue: the match distance refers to the fixed decoded size
			// the vector will vary in size during push backs
			// hence we must store the "start" value of the vector
			// otherwise the offset will be wrong.

		// 2st issue: assert match_distance NOT zero

		if (!match_distance)
			return;

		vector<uint8_t> run;
		int starting_point = _decoded.size() - match_distance;
		while (match_len > match_distance) {
			for (int index = starting_point; index < _decoded.size(); ++index) {
				run.push_back(_decoded[index]);
			}
			match_len -= match_distance;
		}

		for (int index = starting_point; index < starting_point + match_len; ++index) {
			run.push_back(_decoded[index]);
		}

		for (auto r : run)
			appendLiteral(r);
		//for (uint8_t i = 0; i < match_len; i++) {

		//	uint8_t literal = literalByCircularIndex(i, match_distance, initDecodedSize);
		//	//uint8_t literal = _decoded[initDecodedSize - match_distance + i];
		//	appendLiteral(literal);

		//}
	}




	bool decode() {
		if (!readHeader())
			return false;
		go_decode();
	};


	/* in c++ you can only call the constructor of your immediate parent */
	struct opcode {
		uint8_t _first_byte;
		string _type = "";
		lzvn& _l;

		opcode(uint8_t first_byte, lzvn& l) : _l(l) {
			_first_byte = first_byte;

			uint8_t first_four_bits = (_first_byte >> 4);
			uint8_t last_three_bits = (_first_byte & 0b0000'0111);
			uint8_t first_three_bits = (_first_byte & 0b1110'0000);




			map<uint8_t, string> h =
			  { { 0b00000000, "sml_d"},
				{ 0b00000001, 	"sml_d" },
				{ 0b00000010, 	"sml_d" },
				{ 0b00000011, 	"sml_d" },
				{ 0b00000100, 	"sml_d" },
				{ 0b00000101, 	"sml_d" },
				{ 0b00000110, 	"eos" },
				{ 0b00000111, 	"lrg_d" },
				{ 0b00001000, 	"sml_d" },
				{ 0b00001001, 	"sml_d" },
				{ 0b00001010, 	"sml_d" },
				{ 0b00001011, 	"sml_d" },
				{ 0b00001100, 	"sml_d" },
				{ 0b00001101, 	"sml_d" },
				{ 0b00001110, 	"nop" },
				{ 0b00001111, 	"lrg_d" },
				{ 0b00010000, 	"sml_d" },
				{ 0b00010001, 	"sml_d" },
				{ 0b00010010, 	"sml_d" },
				{ 0b00010011, 	"sml_d" },
				{ 0b00010100, 	"sml_d" },
				{ 0b00010101, 	"sml_d" },
				{ 0b00010110, 	"nop" },
				{ 0b00010111, 	"lrg_d" },
				{ 0b00011000, 	"sml_d" },
				{ 0b00011001, 	"sml_d" },
				{ 0b00011010, 	"sml_d" },
				{ 0b00011011, 	"sml_d" },
				{ 0b00011100, 	"sml_d" },
				{ 0b00011101, 	"sml_d" },
				{ 0b00011110, 	"udef" },
				{ 0b00011111, 	"lrg_d" },
				{ 0b00100000, 	"sml_d" },
				{ 0b00100001, 	"sml_d" },
				{ 0b00100010, 	"sml_d" },
				{ 0b00100011, 	"sml_d" },
				{ 0b00100100, 	"sml_d" },
				{ 0b00100101, 	"sml_d" },
				{ 0b00100110, 	"udef" },
				{ 0b00100111, 	"lrg_d" },
				{ 0b00101000, 	"sml_d" },
				{ 0b00101001, 	"sml_d" },
				{ 0b00101010, 	"sml_d" },
				{ 0b00101011, 	"sml_d" },
				{ 0b00101100, 	"sml_d" },
				{ 0b00101101, 	"sml_d" },
				{ 0b00101110, 	"udef" },
				{ 0b00101111, 	"lrg_d" },
				{ 0b00110000, 	"sml_d" },
				{ 0b00110001, 	"sml_d" },
				{ 0b00110010, 	"sml_d" },
				{ 0b00110011, 	"sml_d" },
				{ 0b00110100, 	"sml_d" },
				{ 0b00110101, 	"sml_d" },
				{ 0b00110110, 	"udef" },
				{ 0b00110111, 	"lrg_d" },
				{ 0b00111000, 	"sml_d" },
				{ 0b00111001, 	"sml_d" },
				{ 0b00111010, 	"sml_d" },
				{ 0b00111011, 	"sml_d" },
				{ 0b00111100, 	"sml_d" },
				{ 0b00111101, 	"sml_d" },
				{ 0b00111110, 	"udef" },
				{ 0b00111111, 	"lrg_d" },
				{ 0b01000000, 	"sml_d" },
				{ 0b01000001, 	"sml_d" },
				{ 0b01000010, 	"sml_d" },
				{ 0b01000011, 	"sml_d" },
				{ 0b01000100, 	"sml_d" },
				{ 0b01000101, 	"sml_d" },
				{ 0b01000110, 	"pre_d" },
				{ 0b01000111, 	"lrg_d" },
				{ 0b01001000, 	"sml_d" },
				{ 0b01001001, 	"sml_d" },
				{ 0b01001010, 	"sml_d" },
				{ 0b01001011, 	"sml_d" },
				{ 0b01001100, 	"sml_d" },
				{ 0b01001101, 	"sml_d" },
				{ 0b01001110, 	"pre_d" },
				{ 0b01001111, 	"lrg_d" },
				{ 0b01010000, 	"sml_d" },
				{ 0b01010001, 	"sml_d" },
				{ 0b01010010, 	"sml_d" },
				{ 0b01010011, 	"sml_d" },
				{ 0b01010100, 	"sml_d" },
				{ 0b01010101, 	"sml_d" },
				{ 0b01010110, 	"pre_d" },
				{ 0b01010111, 	"lrg_d" },
				{ 0b01011000, 	"sml_d" },
				{ 0b01011001, 	"sml_d" },
				{ 0b01011010, 	"sml_d" },
				{ 0b01011011, 	"sml_d" },
				{ 0b01011100, 	"sml_d" },
				{ 0b01011101, 	"sml_d" },
				{ 0b01011110, 	"pre_d" },
				{ 0b01011111, 	"lrg_d" },
				{ 0b01100000, 	"sml_d" },
				{ 0b01100001, 	"sml_d" },
				{ 0b01100010, 	"sml_d" },
				{ 0b01100011, 	"sml_d" },
				{ 0b01100100, 	"sml_d" },
				{ 0b01100101, 	"sml_d" },
				{ 0b01100110, 	"pre_d" },
				{ 0b01100111, 	"lrg_d" },
				{ 0b01101000, 	"sml_d" },
				{ 0b01101001, 	"sml_d" },
				{ 0b01101010, 	"sml_d" },
				{ 0b01101011, 	"sml_d" },
				{ 0b01101100, 	"sml_d" },
				{ 0b01101101, 	"sml_d" },
				{ 0b01101110, 	"pre_d" },
				{ 0b01101111, 	"lrg_d" },
				{ 0b01110000, 	"udef" },
				{ 0b01110001, 	"udef" },
				{ 0b01110010, 	"udef" },
				{ 0b01110011, 	"udef" },
				{ 0b01110100, 	"udef" },
				{ 0b01110101, 	"udef" },
				{ 0b01110110, 	"udef" },
				{ 0b01110111, 	"udef" },
				{ 0b01111000, 	"udef" },
				{ 0b01111001, 	"udef" },
				{ 0b01111010, 	"udef" },
				{ 0b01111011, 	"udef" },
				{ 0b01111100, 	"udef" },
				{ 0b01111101, 	"udef" },
				{ 0b01111110, 	"udef" },
				{ 0b01111111, 	"udef" },
				{ 0b10000000, 	"sml_d" },
				{ 0b10000001, 	"sml_d" },
				{ 0b10000010, 	"sml_d" },
				{ 0b10000011, 	"sml_d" },
				{ 0b10000100, 	"sml_d" },
				{ 0b10000101, 	"sml_d" },
				{ 0b10000110, 	"pre_d" },
				{ 0b10000111, 	"lrg_d" },
				{ 0b10001000, 	"sml_d" },
				{ 0b10001001, 	"sml_d" },
				{ 0b10001010, 	"sml_d" },
				{ 0b10001011, 	"sml_d" },
				{ 0b10001100, 	"sml_d" },
				{ 0b10001101, 	"sml_d" },
				{ 0b10001110, 	"pre_d" },
				{ 0b10001111, 	"lrg_d" },
				{ 0b10010000, 	"sml_d" },
				{ 0b10010001, 	"sml_d" },
				{ 0b10010010, 	"sml_d" },
				{ 0b10010011, 	"sml_d" },
				{ 0b10010100, 	"sml_d" },
				{ 0b10010101, 	"sml_d" },
				{ 0b10010110, 	"pre_d" },
				{ 0b10010111, 	"lrg_d" },
				{ 0b10011000, 	"sml_d" },
				{ 0b10011001, 	"sml_d" },
				{ 0b10011010, 	"sml_d" },
				{ 0b10011011, 	"sml_d" },
				{ 0b10011100, 	"sml_d" },
				{ 0b10011101, 	"sml_d" },
				{ 0b10011110, 	"pre_d" },
				{ 0b10011111, 	"lrg_d" },
				{ 0b10100000, 	"med_d" },
				{ 0b10100001, 	"med_d" },
				{ 0b10100010, 	"med_d" },
				{ 0b10100011, 	"med_d" },
				{ 0b10100100, 	"med_d" },
				{ 0b10100101, 	"med_d" },
				{ 0b10100110, 	"med_d" },
				{ 0b10100111, 	"med_d" },
				{ 0b10101000, 	"med_d" },
				{ 0b10101001, 	"med_d" },
				{ 0b10101010, 	"med_d" },
				{ 0b10101011, 	"med_d" },
				{ 0b10101100, 	"med_d" },
				{ 0b10101101, 	"med_d" },
				{ 0b10101110, 	"med_d" },
				{ 0b10101111, 	"med_d" },
				{ 0b10110000, 	"med_d" },
				{ 0b10110001, 	"med_d" },
				{ 0b10110010, 	"med_d" },
				{ 0b10110011, 	"med_d" },
				{ 0b10110100, 	"med_d" },
				{ 0b10110101, 	"med_d" },
				{ 0b10110110, 	"med_d" },
				{ 0b10110111, 	"med_d" },
				{ 0b10111000, 	"med_d" },
				{ 0b10111001, 	"med_d" },
				{ 0b10111010, 	"med_d" },
				{ 0b10111011, 	"med_d" },
				{ 0b10111100, 	"med_d" },
				{ 0b10111101, 	"med_d" },
				{ 0b10111110, 	"med_d" },
				{ 0b10111111, 	"med_d" },
				{ 0b11000000, 	"sml_d" },
				{ 0b11000001, 	"sml_d" },
				{ 0b11000010, 	"sml_d" },
				{ 0b11000011, 	"sml_d" },
				{ 0b11000100, 	"sml_d" },
				{ 0b11000101, 	"sml_d" },
				{ 0b11000110, 	"pre_d" },
				{ 0b11000111, 	"lrg_d" },
				{ 0b11001000, 	"sml_d" },
				{ 0b11001001, 	"sml_d" },
				{ 0b11001010, 	"sml_d" },
				{ 0b11001011, 	"sml_d" },
				{ 0b11001100, 	"sml_d" },
				{ 0b11001101, 	"sml_d" },
				{ 0b11001110, 	"pre_d" },
				{ 0b11001111, 	"lrg_d" },
				{ 0b11010000, 	"udef" },
				{ 0b11010001, 	"udef" },
				{ 0b11010010, 	"udef" },
				{ 0b11010011, 	"udef" },
				{ 0b11010100, 	"udef" },
				{ 0b11010101, 	"udef" },
				{ 0b11010110, 	"udef" },
				{ 0b11010111, 	"udef" },
				{ 0b11011000, 	"udef" },
				{ 0b11011001, 	"udef" },
				{ 0b11011010, 	"udef" },
				{ 0b11011011, 	"udef" },
				{ 0b11011100, 	"udef" },
				{ 0b11011101, 	"udef" },
				{ 0b11011110, 	"udef" },
				{ 0b11011111, 	"udef" },
				{ 0b11100000, 	"lrg_l" },
				{ 0b11100001, 	"sml_l" },
				{ 0b11100010, 	"sml_l" },
				{ 0b11100011, 	"sml_l" },
				{ 0b11100100, 	"sml_l" },
				{ 0b11100101, 	"sml_l" },
				{ 0b11100110, 	"sml_l" },
				{ 0b11100111, 	"sml_l" },
				{ 0b11101000, 	"sml_l" },
				{ 0b11101001, 	"sml_l" },
				{ 0b11101010, 	"sml_l" },
				{ 0b11101011, 	"sml_l" },
				{ 0b11101100, 	"sml_l" },
				{ 0b11101101, 	"sml_l" },
				{ 0b11101110, 	"sml_l" },
				{ 0b11101111, 	"sml_l" },
				{ 0b11110000, 	"lrg_m" },
				{ 0b11110001, 	"sml_m" },
				{ 0b11110010, 	"sml_m" },
				{ 0b11110011, 	"sml_m" },
				{ 0b11110100, 	"sml_m" },
				{ 0b11110101, 	"sml_m" },
				{ 0b11110110, 	"sml_m" },
				{ 0b11110111, 	"sml_m" },
				{ 0b11111000, 	"sml_m" },
				{ 0b11111001, 	"sml_m" },
				{ 0b11111010, 	"sml_m" },
				{ 0b11111011, 	"sml_m" },
				{ 0b11111100, 	"sml_m" },
				{ 0b11111101, 	"sml_m" },
				{ 0b11111110, 	"sml_m" },
				{ 0b11111111, 	"sml_m" }
};
			_type = h[first_byte];

			return;

			/*if (_first_byte == 0b11100000) {
				_type = "lrg_l";
			} else {
				if (_first_byte == 0b11110000) {
					_type = "lrg_m";
				} else {
					if (first_four_bits == 0b1110) {
						_type = "sml_l";
					} else {
						if (first_four_bits == 0b1111) {
							_type = "sml_m";
						} else {
							if (last_three_bits == 0b110) {
								_type = "pre_d";
							} else {
								if (last_three_bits == 0b111) {
									_type = "lrg_d";
								} else {
									if ((first_three_bits >> 5) == 0b101) {
										_type = "med_d";
									} else {
										if (first_byte == 0b00111110 || first_byte == 0b00110110 || first_byte == 0b00101110 || first_byte == 0b00100110 || first_byte == 0b00011110) {
											_type = "udef";
										} else {
											if (first_four_bits == 0b0111 || first_four_bits == 0b1101) {
												_type = "udef";
											} else {
												if (first_byte == 0b00001110 || first_byte == 0b00010110) {
													_type = "nop";
												} else {
													if (first_byte == 0b00000110) {
														_type = "eos";
													} else {
														if (first_byte == 0b00001110 || first_byte == 0b00010110) {
															_type = "nop";
														} else {
															_type = "sml_d";
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}*/
		}

		string getType() {
			return _type;
		}

		uint8_t readNextByte() {
			return _l.readNextByte();
		}


		void action() {
			uint8_t literal_len = 0, match_len = 0;
			uint16_t match_distance = 0;
			if (_type == "sml_d") {

				literal_len = (_first_byte >> 6) & 0b0000'0011;


				match_len = (_first_byte >> 3) & 0b0000'0111;
				match_len += 3;		// adding ml bias

				match_distance = _first_byte & 0b0000'0111;
				match_distance <<= 8;
				uint8_t next = this->readNextByte();
				match_distance |= next;



				_l.simpleAppendNLiterals(literal_len);

				_l.updateLatestMatchDistance(match_distance);
				_l.addMatchToDecoded(match_len, match_distance);


			}
			if (_type == "sml_l") {
				
				literal_len = _first_byte & 0b0000'1111;
				_l.simpleAppendNLiterals(literal_len);

			}

			if (_type == "sml_m") {
				match_len = _first_byte & 0b0000'1111;
				match_distance = _l.getLatestMatchDistance();
				_l.addMatchToDecoded(match_len, match_distance);
			}

			if (_type == "lrg_m") {
				match_len = readNextByte();
				match_len += 16;		// adding bias
				match_distance = _l.getLatestMatchDistance();
				_l.addMatchToDecoded(match_len, match_distance);
			}

			if (_type == "pre_d") {
				literal_len = (_first_byte >> 6) & 0b0000'0011;
				match_len = (_first_byte >> 3) & 0b0000'0111;
				match_len += 3;		// adding bias
				_l.simpleAppendNLiterals(literal_len);
				_l.addMatchToDecoded(match_len, _l.getLatestMatchDistance());
			}


			if (_type == "lrg_l") {
				uint8_t next = readNextByte();
				literal_len |= next;
				literal_len += 16;		// adding bias
				_l.simpleAppendNLiterals(literal_len);

			}

			if (_type == "med_d") {
				literal_len = (_first_byte >> 3) & 0b0000'0011;
				match_len = (_first_byte & 0b00000111);
				match_len <<= 2;

				uint8_t next = readNextByte();
				uint8_t m = 0b0000'0011;
				match_len |= (next & m);
				match_len += 3;		// adding bias

				//match_distance = ((next & ~m) >> 2);



				uint8_t next_next = readNextByte();
				//match_distance += next_next;
				match_distance = next_next;
																// left shift by six since I have to emplace 6 bits not 8!
				match_distance <<= 6;							// ACHTUNG ACHTUNG ACHTUNG ACHTUNG ACHTUNG ACHTUNG ACHTUNG 
																
				uint8_t remain = (next >> 2) & 0b0011'1111;
				match_distance += remain;
				//match_distance |= ((next & 0b1111'1100) >> 2);

				_l.simpleAppendNLiterals(literal_len);

				_l.updateLatestMatchDistance(match_distance);
				_l.addMatchToDecoded(match_len, match_distance);
			}

			if (_type == "lrg_d") {
				literal_len = (_first_byte >> 6) & 0b0000'0011;
				match_len = (_first_byte >> 3) & 0b0000'0111;
				match_len += 3;		// adding bias

				uint8_t next = readNextByte();
				uint8_t next_next = readNextByte();
				match_distance = (next_next << 8);
				match_distance |= next;
				//match_distance = next;
				//match_distance <<= 8;
				//match_distance |= next_next;

				_l.simpleAppendNLiterals(literal_len);

				_l.updateLatestMatchDistance(match_distance);
				_l.addMatchToDecoded(match_len, match_distance);
			}


			if (_type == "nop")
				return;


		}

	};




};




int main(int argc, char** argv) {


	ifstream in(argv[1], ios::binary);
	ofstream os(argv[2], ios::binary);

	lzvn l(in, os);

	l.decode();

	return EXIT_SUCCESS;
}