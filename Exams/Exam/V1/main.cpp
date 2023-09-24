#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <array>
#include <deque>
using namespace std;


// PIXEL_GREY_VALUE, PIXEL_LEVEL
using adampixel = array <uint8_t, 2>;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T>
ostream& raw_write(ostream& out, const T& v) {
	return out.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

class pgm_out {
private:
	ostream& _out;
	int _width, _height, _maxval;


public:
	pgm_out(ostream& out, int width, int heigth) : _out(out), _width(width), _height(heigth) {};

	void writeHeader() {
		_out << "P5\n";
		_out << _width << "\n";
		_out << _height << "\n";
		_out << "255\n";
	}


	void saveImage(vector<vector<adampixel>>& data) {
		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				try {
					uint8_t car = data.at(r).at(c)[0];
					raw_write(_out, car);
				} catch (std::out_of_range const& exc) {
					cout << "out of bound exception pgm saveImage" << endl;
				};
			}
		}
	}

	void saveImage(vector<vector<uint8_t>>& data) {
		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				try {
					uint8_t car = data.at(r).at(c);
					raw_write(_out, car);
				} catch (std::out_of_range const& exc) {
					cout << "out of bound exception pgm saveImage" << endl;
				};
			}
		}
	}


	auto getWidth() {
		return this->_width;
	}

	auto getHeigth() {
		return this->_height;
	}

};


class pgm {
private:
	istream& _in;
	int _width, _height, _maxval;
	vector<vector<uint8_t>> _data;

public:

	pgm(istream& in) : _in(in) {
		this->readHeader();
	};

	void writeHeader(ostream& out) {
		out << "P5\n";
		out << _width << "\n";
		out << _height << "\n";
		out << "255\n";
	}

	bool readHeader() {
		string mw;
		_in >> mw;
		if (mw != "P5") {
			return false;
		}
		_in.ignore(); // \n
		uint8_t b = 0;
		if (_in.peek() == '#') {
			while (_in.get() != '\n');
		}

		_in >> _width;
		_in >> _height;
		_in >> _maxval;
		_in.ignore();	// \n
		if (!_in)
			return false;

		return true;

	}

	void readData() {
		for (int r = 0; r < _height; r++) {
			vector<uint8_t> row;
			for (int c = 0; c < _width; c++) {
				uint8_t b = 0;
				if (!raw_read(_in, b)) {
					cout << "error raw_read" << endl;
				}
				row.push_back(b);
			}
			_data.push_back(row);
		}
	}

	void saveImage(ostream& out) {
		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				try {
					raw_write(out, _data.at(r).at(c));
				} catch (std::out_of_range const& exc) {
					cout << "out of bound exception pgm saveImage" << endl;
				};
			}
		}
	}

	void saveImage(ostream& out, vector<vector<uint8_t>>& data) {
		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				try {
					uint8_t c = data.at(r).at(c);
					raw_write(out, c);
				} catch (std::out_of_range const& exc) {
					cout << "out of bound exception pgm saveImage" << endl;
				};
			}
		}
	}

	auto getData() {
		return this->_data;
	}


	auto getWidth() {
		return this->_width;
	}

	auto getHeigth() {
		return this->_height;
	}

};




class adam {
private:
	uint32_t _width, _height;
	ostream& _out;
	vector<vector<adampixel>> _AdamData;

	vector<vector<uint8_t>> _adamKernel;


	void buildAdamKernel() {
		// probabilmente più opportuno un vector<array<>>

		vector<uint8_t> adamRow = { 1,6,4,6,2,6,4,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7};
		_adamKernel.push_back(adamRow);

		adamRow = { 5,6,5,6,5,6,5,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

		adamRow = { 3,6,4,6,3,6,4,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

		adamRow = { 5,6,5,6,5,6,5,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

	}

	bool writeHeader(){
		string mw = "MULTIRES";
		_out.write(&mw[0], 8);
		if (!raw_write(_out, _width)) {
			return false;
		}
		if (!raw_write(_out, _height)) {
			return false;
		}

		return true;
	}

public:

	adam(uint32_t width, uint32_t height, ostream& out) : _out(out), _width(width), _height(height) {
		this->buildAdamKernel();
		if (!this->writeHeader()) {
			cout << "error adamheader" << endl;
		}
		
	};


	uint8_t getAdamLavelByPosition(int r, int c) {
		return _adamKernel[r % 8][c % 8];

	}

	void loadData(vector<vector<uint8_t>>& data) {

		for (int r = 0; r < _height; r++) {
			vector<adampixel> row;
			for (int c = 0; c < _width; c++) {
				adampixel adamp = {data[r][c],0};
				row.push_back(adamp);

			}
			_AdamData.push_back(row);
		}
	}

	void labelAdamPixels() {
		

		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				_AdamData[r][c][1] = getAdamLavelByPosition(r, c);
			}
		}

	}

	void writeOutAdamLevels() {

		for (uint8_t level = 0; level < 8; level++) {
			for (int r = 0; r < _height; r++) {
				for (int c = 0; c < _width; c++) {
					//cout << "writing adam level " << level + '0' << endl;
					if (_AdamData[r][c][1] == level) {
						if (!raw_write(_out, _AdamData[r][c][0])) {
							cout << "error raw_write writeOutAdamLevels at level: " << level << endl;
						}
					}
				}
			}
		}

	}


};



// super ugly solution but i dont have the time to understand how to null instantiate 
	// a istream/ostream reference, can be solved by pointers but dont have time to refractor the encode
class adam_decode {
private:
	vector<vector<adampixel>> _AdamData;	
	vector<vector<uint8_t>> _adamKernel;
	uint32_t _width, _height;
	ifstream& _in;
	array<size_t, 7> _howManyByLevel = { 0,0,0,0,0,0,0 };
	vector<deque<uint8_t>> _AdamLevelsRaw;	// deque absolutely overkill but saves time & speeds up debugging for me
	string _prefix = "";

	void buildAdamKernel() {
		// probabilmente più opportuno un vector<array<>>

		vector<uint8_t> adamRow = { 1,6,4,6,2,6,4,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

		adamRow = { 5,6,5,6,5,6,5,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

		adamRow = { 3,6,4,6,3,6,4,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

		adamRow = { 5,6,5,6,5,6,5,6 };
		_adamKernel.push_back(adamRow);

		adamRow = { 7,7,7,7,7,7,7,7 };
		_adamKernel.push_back(adamRow);

	}

	uint8_t getAdamLavelByPosition(int r, int c) {
		return _adamKernel[r % 8][c % 8];

	}




public:
	adam_decode(ifstream& in, string prefix) : _in(in), _prefix(prefix) {
		this->buildAdamKernel();

	};


	bool readHeader() {
		string mw = "        ";
		_in.read(&mw[0], 8);

		if (mw != "MULTIRES")
			return false;
		if (!raw_read(_in, _width)) {
			return false;
		}

		if (!raw_read(_in, _height)) {
			return false;
		}

		for (int r = 0; r < _height; r++) {
			vector<adampixel> row(_width, { 0,0 });
			_AdamData.push_back(row);
		}

		return true;
	}



	void labelAdamPixels() {
		for (int r = 0; r < _height; r++) {
			for (int c = 0; c < _width; c++) {
				_AdamData[r][c][1] = getAdamLavelByPosition(r, c);
			}
		}
	}

	void computeHowManyByLevel() {

		for (uint8_t level = 0; level < 7; level++) {

			size_t count = 0;
			for (int r = 0; r < _height; r++) {
				for (int c = 0; c < _width; c++) {
					if ((level+1) == _AdamData[r][c][1])
						_howManyByLevel[level]++;
				}

			}
		}

	}

	void decodeLevelSeven() {
		vector<deque<uint8_t>> LevelsRaw; // si può usare un vector facilmente, o template Tvec per un container generico

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}


		for (int r = 0; r < _height; r++) {

			for (int c = 0; c < _width; c++) {

				uint8_t this_level = getAdamLavelByPosition(r, c);
				uint8_t this_byte = LevelsRaw[this_level - 1].front();
				LevelsRaw[this_level - 1].pop_front();
				_AdamData[r][c][0] = this_byte;
				_AdamData[r][c][1] = this_level;
			}
		}

		string out_name = _prefix;
		out_name += "_7.pgm";
		ofstream out(out_name);

		pgm_out seven(out, _width, _height);
		seven.writeHeader();
		seven.saveImage(_AdamData);
	}


	void decodeLevelOne() {
		vector<deque<uint8_t>> LevelsRaw;

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}

		size_t padded_height = _height + (8 - (_height % 8));
		size_t padded_width = _width + (8 - (_width % 8));

		vector<vector<uint8_t>> image(padded_height);
		for (int i = 0; i < padded_height; i++) {
			vector<uint8_t> row(padded_width);
			image[i]=row;
		}

		size_t col_shifts = 0, row_shifts=0;
		int row = 0;

		while (true) {

			if (LevelsRaw[0].size() == 0) {
				break;
			}

			uint8_t byte = LevelsRaw[0].front();
			LevelsRaw[0].pop_front();

			int col = 0;

			if (col_shifts == padded_width) {
				row_shifts += 8;
				col_shifts = 0;
			}

			for (; col < 8; col++) {
				for (int row = 0; row < 8; row++) {
					image[row + row_shifts][col+col_shifts] = byte;
				}
			}

			col_shifts+=8;

		}


		string out_name = _prefix;
		out_name += "_1.pgm";
		ofstream out(out_name);

		pgm_out first(out, _width, _height);
		first.writeHeader();
		first.saveImage(image);

	}

	void decodeLevelTwo() {
		vector<deque<uint8_t>> LevelsRaw;

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}

		size_t padded_height = _height + (8 - (_height % 8));
		size_t padded_width = _width + (8 - (_width % 8));

		vector<vector<uint8_t>> image(padded_height);
		for (int i = 0; i < padded_height; i++) {
			vector<uint8_t> row(padded_width);
			image[i] = row;
		}

		size_t col_shifts = 0, row_shifts = 0;
		int row = 0;

		while (true) {

			if (LevelsRaw[1].size() == 0) {
				break;
			}

			uint8_t byte = LevelsRaw[1].front();
			LevelsRaw[1].pop_front();


			int col = 0;

			if (col_shifts == padded_width) {
				row_shifts += 8;
				col_shifts = 0;
			}

			for (; col < 4; col++) {
				for (int row = 0; row < 8; row++) {
					image[row + row_shifts][col + col_shifts] = byte;
				}
			}

			col_shifts += 4;

		}


		string out_name = _prefix;
		out_name += "_2.pgm";
		ofstream out(out_name);

		pgm_out first(out, _width, _height);
		first.writeHeader();
		first.saveImage(image);

	}


	void decodeLevelThree() {
		vector<deque<uint8_t>> LevelsRaw;

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}

		size_t padded_height = _height + (8 - (_height % 8));
		size_t padded_width = _width + (8 - (_width % 8));

		vector<vector<uint8_t>> image(padded_height);
		for (int i = 0; i < padded_height; i++) {
			vector<uint8_t> row(padded_width);
			image[i] = row;
		}

		size_t col_shifts = 0, row_shifts = 0;
		int row = 0;

		while (true) { // while(size != 0)

			if (LevelsRaw[2].size() == 0) {
				break;
			}

			uint8_t byte = LevelsRaw[2].front();
			LevelsRaw[2].pop_front();


			int col = 0;

			if (col_shifts == padded_width) {
				row_shifts += 4;
				col_shifts = 0;
			}

			for (; col < 4; col++) {
				for (int row = 0; row < 4; row++) {
					image[row + row_shifts][col + col_shifts] = byte;
				}
			}

			col_shifts += 4;

		}


		string out_name = _prefix;
		out_name += "_3.pgm";
		ofstream out(out_name);

		pgm_out three(out, _width, _height);
		three.writeHeader();
		three.saveImage(image);

	}



	void decodeLevelFour() {
		vector<deque<uint8_t>> LevelsRaw;

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}

		size_t padded_height = _height + (8 - (_height % 8));
		size_t padded_width = _width + (8 - (_width % 8));

		vector<vector<uint8_t>> image(padded_height);
		for (int i = 0; i < padded_height; i++) {
			vector<uint8_t> row(padded_width);
			image[i] = row;
		}

		size_t col_shifts = 0, row_shifts = 0;
		int row = 0;

		while (true) {

			if (LevelsRaw[3].size() == 0) {
				break;
			}

			uint8_t byte = LevelsRaw[3].front();
			LevelsRaw[3].pop_front();


			int col = 0;

			if (col_shifts == padded_width) {
				row_shifts += 4;
				col_shifts = 0;
			}

			for (; col < 2; col++) {
				for (int row = 0; row < 4; row++) {
					image[row + row_shifts][col + col_shifts] = byte;
				}
			}

			col_shifts += 2;

		}


		string out_name = _prefix;
		out_name += "_4.pgm";
		ofstream out(out_name);

		pgm_out four(out, _width, _height);
		four.writeHeader();
		four.saveImage(image);

	}

	void decodeLevelFive() {
		vector<deque<uint8_t>> LevelsRaw;

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}

		size_t padded_height = _height + (8 - (_height % 8));
		size_t padded_width = _width + (8 - (_width % 8));

		vector<vector<uint8_t>> image(padded_height);
		for (int i = 0; i < padded_height; i++) {
			vector<uint8_t> row(padded_width);
			image[i] = row;
		}

		size_t col_shifts = 0, row_shifts = 0;
		int row = 0;

		while (true) {

			if (LevelsRaw[4].size() == 0) {
				break;
			}

			uint8_t byte = LevelsRaw[4].front();
			LevelsRaw[4].pop_front();

			int col = 0;

			if (col_shifts == padded_width) {
				row_shifts += 2;
				col_shifts = 0;
			}

			for (; col < 2; col++) {
				for (int row = 0; row < 2; row++) {
					image[row + row_shifts][col + col_shifts] = byte;
				}
			}

			col_shifts += 2;

		}


		string out_name = _prefix;
		out_name += "_5.pgm";
		ofstream out(out_name);

		pgm_out five(out, _width, _height);
		five.writeHeader();
		five.saveImage(image);

	}

	void decodeLevelSix() {
		vector<deque<uint8_t>> LevelsRaw;

		for (uint8_t level = 0; level < 7; level++) {
			deque<uint8_t> row;
			for (auto e : _AdamLevelsRaw[level])
				row.push_back(e);
			LevelsRaw.push_back(row);
		}

		size_t padded_height = _height + (8 - (_height % 8));
		size_t padded_width = _width + (8 - (_width % 8));

		vector<vector<uint8_t>> image(padded_height);
		for (int i = 0; i < padded_height; i++) {
			vector<uint8_t> row(padded_width);
			image[i] = row;
		}

		size_t col_shifts = 0, row_shifts = 0;
		int row = 0;

		while (true) {

			if (LevelsRaw[6].size() == 0) {
				break;
			}

			uint8_t byte = LevelsRaw[6].front();
			LevelsRaw[6].pop_front();


			int col = 0;

			if (col_shifts == padded_width) {
				row_shifts += 2;
				col_shifts = 0;
			}

			for (; col < 1; col++) {
				for (int row = 0; row < 2; row++) {
					image[row + row_shifts][col + col_shifts] = byte;
				}
			}

			col_shifts += 1;

		}


		string out_name = _prefix;
		out_name += "_6.pgm";
		ofstream out(out_name);

		pgm_out six(out, _width, _height);
		six.writeHeader();
		six.saveImage(image);

	}


	void readRawAdamLevels() {

		for (int level = 1; level <= 8; level++) {
			deque<uint8_t> rowLevelData;
			if (level == 8)
				break;
			for (size_t cursor = 0; cursor < _howManyByLevel[level-1]; cursor++) {
				uint8_t b = 0;
				if (!raw_read(_in, b)) {
					cout << "error readRawAdamLevels " << endl;
				}
				rowLevelData.push_back(b);
			}

			_AdamLevelsRaw.push_back(rowLevelData);

		}

	}


	bool checkLevelAviab(uint8_t level) {
		return _howManyByLevel[level - 1] > 0 ? true : false;
	}



	
};


int main(int argc, char** argv) {
	
	if (argc != 4) {
		cout << "Usage: " << endl;
		return EXIT_FAILURE;
	};

	char option = *argv[1];
	if (option == 'c') {
		ifstream in(argv[2], ios::binary);
		ofstream pgm_out_test("lalala.pgm", ios::binary);

		if (!in) {
			return EXIT_FAILURE;
		}
		pgm input_image(in);
		input_image.readData();

		input_image.writeHeader(pgm_out_test);
		input_image.saveImage(pgm_out_test);

		ofstream adam_out(argv[3], ios::binary);
		if (!adam_out)
			return EXIT_FAILURE;
		
		adam a(input_image.getWidth(), input_image.getHeigth(), adam_out);
		auto data = input_image.getData();
		a.loadData(data);
		a.labelAdamPixels();
		a.writeOutAdamLevels();


	} else {
		if (option == 'd') {
		
			ifstream input_adam_image(argv[2], ios::binary);
			if (!input_adam_image) {
				cout << "unable open adam .mlt" << endl;
				return EXIT_FAILURE;
			}

			adam_decode a(input_adam_image, argv[3]);
			if (!a.readHeader()) {
				cout << "error reading adam header" << endl;
				return EXIT_FAILURE;
			}
			a.labelAdamPixels();
			a.computeHowManyByLevel();
			a.readRawAdamLevels();

			// the decodeLevelN can(had to) be generalized as a function
				// fatto solo per capire meglio perchè dimezzasse ad ogni livello

			if(a.checkLevelAviab(7))
				a.decodeLevelSeven();

			if(a.checkLevelAviab(1))
				a.decodeLevelOne();

			if (a.checkLevelAviab(2))
				a.decodeLevelTwo();

			if (a.checkLevelAviab(3))
				a.decodeLevelThree();
			
			if (a.checkLevelAviab(4))
				a.decodeLevelFour();

			if (a.checkLevelAviab(5))
				a.decodeLevelFive();

			if (a.checkLevelAviab(6))
				a.decodeLevelSix();




		
		} else {
			return EXIT_FAILURE;
		}
	
	}

	return EXIT_SUCCESS;
}