#include <fstream>
#include <cstdint>
#include <iostream>
#include <array>
#include <vector>

using namespace std;

template <typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
};

template <typename T>
ostream& raw_write(ostream& out, const T& v) {
	return out.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

class pam {
	using rgb = array <uint8_t, 3>;
	using pixel = array <uint8_t, 3>;

private:
	ostream& _out;
	int _width, _height, _depth, _maxval;
	vector<rgb> _data;
public:

	pam(ostream& out, int w, int h, int d) : _out(out),_width(w), _height(h), _depth(3), _maxval(255)  {};
	/*
	P7
	WIDTH 227
	HEIGHT 149
	DEPTH 3
	MAXVAL 255
	TUPLTYPE RGB
	ENDHDR
	
	*/

	int raw_size() {
		return this->_width * this->_height;
	}


	void write_header() {
		_out << "P7";
		_out << "\nWIDTH\n";
		_out << _width;
		_out << "\nHEIGHT\n";
		_out << _height;
		_out << "\nDEPTH\n";
		_out << _depth;
		_out << "\nMAXVAL\n";
		_out << _maxval;
		_out << "\nTUPLTYPE";
		_out << "\nRGB";
		_out << "\nENDHDR\n";


	}

	void save2d(vector<vector<pixel>> data) {

		vector<pixel> curr;
		while(data.size() > 0){
			curr = data.back();

			for (auto& p : curr) {
				raw_write(_out, p[2]);
				raw_write(_out, p[1]);
				raw_write(_out, p[0]);

			}

			data.pop_back();
		}
	}

	auto& operator() (int r, int c) {
		return _data[r*_width + c];

	}

};

struct bitmap_file_header {
	/*

	0 	2 bytes 	the header field used to identify the BMP file is 0x42 0x4D in hexadecimal, same as BM in ASCII.
	2 	4 bytes 	the size of the BMP file in bytes
	6 	2 bytes 	reserved; actual value depends on the application that creates the image
	8 	2 bytes 	reserved; actual value depends on the application that creates the image
	10 	4 bytes 	the offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
	*/
	uint32_t bpm_file_size = 0;
	uint16_t first_reserved = 0;
	uint16_t second_reserved = 0;
	uint32_t starting_address = 0;
};

struct bitmap_info_header {
	/*
	14 	4 	the size of this header (40 bytes)
	18 	4 	the bitmap width in pixels (signed integer)
	22 	4 	the bitmap height in pixels (signed integer)
	26 	2 	the number of color planes (must be 1)
	28 	2 	the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32. (bpp)
	30 	4 	the compression method being used. See the next table for a list of possible values
	34 	4 	the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps.
	38 	4 	the horizontal resolution of the image. (pixel per meter, signed integer)
	42 	4 	the vertical resolution of the image. (pixel per meter, signed integer)
	46 	4 	(num_colors) the number of colors in the color palette, or 0 to indicate 2ⁿ, with n the numeber of bits per pixel
	50 	4 	the number of important colors used, or 0 when every color is important; generally ignored
	*/

	uint32_t header_size;
	int32_t bitmap_width;
	int32_t bitmap_heigth;
	uint16_t number_color_planes;
	uint16_t number_bits_per_pixel;
	uint32_t compression_method;
	uint32_t image_size;
	int32_t horizontal_resolution;
	int32_t vertical_resolution;
	uint32_t num_colors;
	uint32_t num_important_colors;

};

using colvec = array<uint8_t, 4>;


class bpm {
private:
	istream& _in;
	ostream& _out;
	bitmap_file_header _bfh = { 0,0,0,0 };
	bitmap_info_header _bih = { 0,0,0,0,0,0,0,0,0,0,0 };
	vector<colvec> _ct;


	bool checkHeader() {

		
		string mw = "  ";
		_in.read(&mw[0], 2);
		if (mw != "BM")
			return false;



		raw_read(_in, _bfh.bpm_file_size);
		raw_read(_in, _bfh.first_reserved);
		raw_read(_in, _bfh.second_reserved);
		raw_read(_in, _bfh.starting_address);
		
		if (!_in)		// maybe better to do after each or even better-> if(raw_read()) {};
			return false;

		raw_read(_in, _bih.header_size);
		if (_bih.header_size != 40)
			return false;

		raw_read(_in, _bih.bitmap_width);
		raw_read(_in, _bih.bitmap_heigth);
		raw_read(_in, _bih.number_color_planes);
		raw_read(_in, _bih.number_bits_per_pixel);
		raw_read(_in, _bih.compression_method);

		/*
		0 	BI_RGB 	none 	Most common
		1 	BI_RLE8 	RLE 8-bit/pixel 	Can be used only with 8-bit/pixel bitmaps
		2 	BI_RLE4 	RLE 4-bit/pixel 	Can be used only with 4-bit/pixel bitmaps
		
		*/

		raw_read(_in, _bih.image_size);
		raw_read(_in, _bih.horizontal_resolution);
		raw_read(_in, _bih.vertical_resolution);
		raw_read(_in, _bih.num_colors);
		_bih.num_colors = _bih.num_colors == 0 ? pow(2, _bih.number_bits_per_pixel) : _bih.num_colors;
		
		raw_read(_in, _bih.num_important_colors);

		if (!_in)
			return false;




		return true;
	}


	void loadColorTable() {

		// SCHEMA: BGR0
		for (size_t i = 0; i < _bih.num_colors; i++) {
			colvec c = { 0,0,0,0 };
			raw_read(_in, c[0]);
			raw_read(_in, c[1]);
			raw_read(_in, c[2]);
			raw_read(_in, c[3]);

			_ct.push_back(c);
		}
	};

	bool bit24perpx() {

		/*
		Then the Color Table follows, i.e. a list of num_colors quadruples of bytes B,G,R,0 for each color. The first corresponds to index 0, the second to index 1 and so on.

		Then the pixels data of the image follow. These are stored on the file with bpp bits for each pixel.
		Pay attention to the fact that:
			1) each row of the image is padded with 0, so that its length is multiple of 32 bits;
			2) the lines are stored from left to right and from bottom to top.*/

			// SCHEMA: BGR0


			// 24 bits per pixel does not have color table


			//}
		using pixel = array<uint8_t, 3>; // RGB
		vector<pixel> img;

		pixel pix = { 0,0,0 };

		size_t row_counter = 0, it=0;
		while(true){

			if (_in.fail()) {
				break;
			}

			if (row_counter == _bih.bitmap_width) {
				while (true) {
					streampos pos = _in.tellg();

					raw_read(_in, pix[0]);
					raw_read(_in, pix[1]);
					raw_read(_in, pix[2]);
					if (_in.peek() != 0x0) {
						row_counter = 0;

						break;
					}
				}
			}

			raw_read(_in, pix[0]);
			raw_read(_in, pix[1]);
			raw_read(_in, pix[2]);
			img.push_back(pix);
			row_counter++;
		}


		// padding eater



		pam p(_out, _bih.bitmap_width, _bih.bitmap_heigth, _bih.number_color_planes);
		// width 375

		vector<vector<pixel>> img2d;
		vector<pixel> v;
		for (int i = 0, index = 0; i < img.size(); i++) {
			if (i % _bih.bitmap_width == 0) {
				img2d.push_back(v);
				v.clear();
				index++;
			}
			v.push_back(img[i]);
		}


		//for (int i = 0; i < toFlip.size(); i++) {

		//}
		p.save2d(img2d);


		return true;
};


public:
	bpm(istream& in, ostream& out) : _in(in), _out(out) {};

	bool go() {
		if (!checkHeader())
			return false;

		bool ret = false;

		switch (_bih.compression_method) {
		case 0:
			ret = bit24perpx();
			break;
		case 1:
			break;

		case 2:
			break;

		default:
			cout << "switch compression method error" << endl;
			break;

		}

		return true;
	}

	



};


int main(int argc, char** argv) {

	if (argc != 3) {
		cout << "Usage: bmp2pam <input file .BMP> <output file .PAM>" << endl;
		return EXIT_FAILURE;
	}

	ifstream in(argv[1], ios::binary);
	ofstream out(argv[2], ios::binary);

	if (!in || !out)
		return EXIT_FAILURE;


	bpm my(in, out);

	my.go();


	return EXIT_SUCCESS;
}