#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <cmath>

void syntax() {
	std::cout << "\n\tSintassi del programma bayer_decode:"
		<< "\n\tbayer_decode <input file .PGM> <output prefix>\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& message) {
	std::cout << "\n\tERRORE: " << message << "\n";
	exit(EXIT_FAILURE);
}

template <typename T>
struct matrix {

	size_t rows_, cols_;
	std::vector<T> data_;

	matrix(){}
	matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols) {
		data_.resize(rows * cols * sizeof(T));
	}
	void resize(size_t rows, size_t cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols * sizeof(T));
	}
	T& operator()(size_t r, size_t c) { return data_[r * cols_ + c]; }
};

struct headerPGM {

	std::string magicNumber;
	size_t width;
	size_t height;
	uint16_t maxval;
};

template <typename T>
struct pgm {

	headerPGM header;
	matrix<T> data_;

	pgm<T>() {}
	pgm<T>(std::istream& is) { readPGM(is); }

	void readHeader(std::istream& is) {
		// Magic number
		is >> header.magicNumber;
		if (header.magicNumber != "P5") { error("File non formattato correttamente."); }
		if(!isspace(is.get())) { error("File non formattato correttamente."); }
		// width
		std::string asciival;
		is >> asciival;
		header.width = stoi(asciival);
		if (is.get() != ' ') { error("File non formattato correttamente."); }
		// height
		is >> asciival;
		header.height = stoi(asciival);
		if (!isspace(is.get())) { error("File non formattato correttamente."); }
		// maxval
		is >> asciival;
		header.maxval = stoi(asciival);
		if (!isspace(is.get())) { error("File non formattato correttamente."); }
	}

	void readData(std::istream& is) {
		// Preparazione dei dati:
		data_.resize(header.height, header.width);
		// Raster:
		for (size_t r = 0; r < data_.rows_; ++r) {
			for (size_t c = 0; c < data_.cols_; ++c) {
				uint16_t pixel;
				is.read(reinterpret_cast<char*>(&pixel), 2);
				data_(r, c) = BEtoLE(pixel);
			}
		}
	}

	uint16_t BEtoLE(uint16_t valueBE) {
		uint16_t valueLE = (valueBE & 0b0000000011111111) << 8;
		valueLE += (valueBE & 0b1111111100000000) >> 8;
		return valueLE;
	}

	void readPGM(std::istream& is) {
		readHeader(is);
		readData(is);
	}

	void pgm8bpp(pgm<uint8_t>& img) {
		img.header.magicNumber = this->header.magicNumber;
		img.header.width = this->header.width;
		img.header.height = this->header.height;
		img.header.maxval = 255;
		img.data_.resize(img.header.height, img.header.width);
		for (size_t r = 0; r < img.data_.rows_; ++r) {
			for (size_t c = 0; c < img.data_.cols_; ++c) {
				img.data_(r, c) = this->data_(r, c) / 256;
			}
		}
	}

	void write_on_file(std::ostream& os) {
		os << header.magicNumber << "\n";
		os << std::to_string(header.width) << " ";
		os << std::to_string(header.height) << " ";
		os << std::to_string(header.maxval) << "\n";
		for (size_t r = 0; r < data_.rows_; ++r) {
			for (size_t c = 0; c < data_.cols_; ++c) {
				os.write(reinterpret_cast<char*>(&data_(r, c)), sizeof(data_(r, c)));
			}
		}
	}
};

struct pixel {
	uint8_t R_, G_, B_;
};

struct headerPPM {
	std::string magicNumber;
	size_t width;
	size_t height;
	uint16_t maxval;
};

struct ppm {

	headerPPM header;
	matrix<pixel> data_;

	void readFromPPM(ppm& input_img, const std::string& output_prefix) {
		header.magicNumber = "P6";
		header.width = input_img.header.width;
		header.height = input_img.header.height;
		header.maxval = input_img.header.maxval;
		bayerDecode(input_img.data_, output_prefix);
	}

	void assingColor(pgm<uint8_t>& input_img) {
		header.magicNumber = "P6";
		header.width = input_img.header.width;
		header.height = input_img.header.height;
		header.maxval = input_img.header.maxval;
		data_.resize(header.height, header.width);
		uint8_t counter = 0;
		for (size_t r = 0; r < data_.rows_; ++r) {
			for (size_t c = 0; c < data_.cols_; ++c) {
				/*
				* RGRGRG
				* GBGBGB
				*/
				if (r % 2 == 0) {
					if (counter == 0) { data_(r, c).R_ = input_img.data_(r, c); }
					if (counter == 1) { data_(r, c).G_ = input_img.data_(r, c); }
					++counter;
					if (counter == 2) { counter = 0; }
				}
				else {
					if (counter == 0) { data_(r, c).G_ = input_img.data_(r, c); }
					if (counter == 1) { data_(r, c).B_ = input_img.data_(r, c); }
					++counter;
					if (counter == 2) { counter = 0; }
				}
			}
			counter = 0;
		}
	}

	void bayerDecode(matrix<pixel>& img_data, const std::string& output_prefix) {
		data_.resize(header.height, header.width);
		data_ = img_data;
		// Per il momento, ignoro la cornice (settata a 0)
		// Prima passata - interpolazione del canale cromatico verde
		size_t dH, dV;
		for (size_t r = 2; r < img_data.rows_ - 2; ++r) {
			for (size_t c = 2; c < img_data.cols_ - 2; ++c) {
				uint8_t G5;
				if (r % 2 == 0) { // RG
					if (c % 2 == 0) {
						dH = abs(data_(r, c - 1).G_ - data_(r, c + 1).G_) + abs(data_(r, c).R_ - data_(r, c - 2).R_ + data_(r, c).R_ - data_(r, c + 2).R_);
						dV = abs(data_(r - 1, c).G_ - data_(r + 1, c).G_) + abs(data_(r, c).R_ - data_(r - 2, c).R_ + data_(r, c).R_ - data_(r + 2, c).R_);
						if (dH < dV) { G5 = saturate(((data_(r, c - 1).G_ + data_(r, c + 1).G_) / 2) + ((data_(r, c).R_ - data_(r, c - 2).R_ + data_(r, c).R_ - data_(r, c + 2).R_) / 4)); }
						if (dH > dV) { G5 = saturate(((data_(r - 1, c).G_ + data_(r + 1, c).G_) / 2) + ((data_(r, c).R_ - data_(r - 2, c).R_ + data_(r, c).R_ - data_(r + 2, c).R_) / 4)); }
						if (dH == dV) { G5 = saturate(((data_(r - 1, c).G_ + data_(r, c - 1).G_ - data_(r, c + 1).G_ + data_(r + 1, c).G_) / 4) + ((data_(r, c).R_ - data_(r - 2, c).R_ + data_(r, c).R_ - data_(r, c - 2).R_ + data_(r, c).R_ - data_(r, c + 2).R_ + data_(r, c).R_ - data_(r + 2, c).R_) / 8)); }
						data_(r, c).G_ = G5;
					}
				}
				else { // GB
					if (c % 2 != 0) {
						dH = abs(data_(r, c - 1).G_ - data_(r, c + 1).G_) + abs(data_(r, c).B_ - data_(r, c - 2).B_ + data_(r, c).B_ - data_(r, c + 2).B_);
						dV = abs(data_(r - 1, c).G_ - data_(r + 1, c).G_) + abs(data_(r, c).B_ - data_(r - 2, c).B_ + data_(r, c).B_ - data_(r + 2, c).B_);
						if (dH < dV) { G5 = saturate(((data_(r, c - 1).G_ + data_(r, c + 1).G_) / 2) + ((data_(r, c).B_ - data_(r, c - 2).B_ + data_(r, c).B_ - data_(r, c + 2).B_) / 4)); }
						if (dH > dV) { G5 = saturate(((data_(r - 1, c).G_ + data_(r + 1, c).G_) / 2) + ((data_(r, c).B_ - data_(r - 2, c).B_ + data_(r, c).B_ - data_(r + 2, c).B_) / 4)); }
						if (dH == dV) { G5 = saturate(((data_(r - 1, c).G_ + data_(r, c - 1).G_ - data_(r, c + 1).G_ + data_(r + 1, c).G_) / 4) + ((data_(r, c).B_ - data_(r - 2, c).B_ + data_(r, c).B_ - data_(r, c - 2).B_ + data_(r, c).B_ - data_(r, c + 2).B_ + data_(r, c).B_ - data_(r + 2, c).B_) / 8)); }
						data_(r, c).G_ = G5;
					}
				}
			}
		}
		std::ofstream os(output_prefix + "_green.ppm", std::ios::binary);
		if (!os) { error("Impossibile aprire il file " + output_prefix + "_green.ppm"); }
		write_on_file(os);
		// Seconda passata
		size_t dN, dP;
		uint8_t X5;
		for (size_t r = 2; r < img_data.rows_ - 2; ++r) {
			for (size_t c = 2; c < img_data.cols_ - 2; ++c) {
				if (r % 2 == 0 && c % 2 == 0) { // 5 e' R, interpolo B
					dN = abs(data_(r - 1, c - 1).B_ - data_(r + 1, c + 1).B_) + abs(data_(r, c).G_ - data_(r - 1, c - 1).G_ + data_(r, c).G_ - data_(r + 1, c + 1).G_);
					dP = abs(data_(r - 1, c + 1).B_ - data_(r + 1, c - 1).B_) + abs(data_(r, c).G_ - data_(r - 1, c + 1).G_ + data_(r, c).G_ - data_(r + 1, c - 1).G_);
					if (dN < dP) { X5 = saturate(((data_(r - 1, c - 1).B_ + data_(r + 1, c + 1).B_)/ 2) + ((data_(r, c).G_ - data_(r - 1, c - 1).G_ + data_(r, c).G_ - data_(r + 1, c + 1).G_)/ 4)); }
					if (dN > dP) { X5 = saturate(((data_(r - 1, c + 1).B_ + data_(r + 1, c - 1).B_) / 2) + ((data_(r, c).G_ - data_(r - 1, c + 1).G_ + data_(r, c).G_ - data_(r + 1, c - 1).G_)/ 4)); }
					if (dN == dP) { X5 = saturate(((data_(r - 1, c - 1).B_ + data_(r - 1, c + 1).B_ + data_(r + 1, c - 1).B_ + data_(r + 1, c + 1).B_)/ 4) + ((data_(r, c).G_ - data_(r - 1, c - 1).G_ + data_(r, c).G_ - data_(r - 1, c + 1).G_ + data_(r, c).G_ - data_(r + 1, c - 1).G_ + data_(r, c).G_ - data_(r + 1, c + 1).G_)/ 8)); }
					data_(r, c).B_ = X5;
				}
				if ((r % 2 == 0 && c % 2 != 0) || (r % 2 != 0 && c % 2 == 0)) { // 5 e' G
					if (r % 2 == 0) { // B sopra e sotto, R destra e sinistra
						data_(r, c).B_ = saturate((data_(r - 1, c).B_ + data_(r + 1, c).B_) / 2);
						data_(r, c).R_ = saturate((data_(r, c - 1).R_ + data_(r, c + 1).R_) / 2);
					}
					else { // R sopra e sotto, B a destra e sinistra
						data_(r, c).R_ = saturate((data_(r - 1, c).R_ + data_(r + 1, c).R_) / 2);
						data_(r, c).B_ = saturate((data_(r, c - 1).B_ + data_(r, c + 1).B_) / 2);
					}
				}
				if (r % 2 != 0 && c % 2 != 0) { // 5 e' B, interpolo R
					dN = abs(data_(r - 1, c - 1).R_ - data_(r + 1, c + 1).R_) + abs(data_(r, c).G_ - data_(r - 1, c - 1).G_ + data_(r, c).G_ - data_(r + 1, c + 1).G_);
					dP = abs(data_(r - 1, c + 1).R_ - data_(r + 1, c - 1).R_) + abs(data_(r, c).G_ - data_(r - 1, c + 1).G_ + data_(r, c).G_ - data_(r + 1, c - 1).G_);
					if (dN < dP) { X5 = saturate(((data_(r - 1, c - 1).R_ + data_(r + 1, c + 1).R_) / 2) + ((data_(r, c).G_ - data_(r - 1, c - 1).G_ + data_(r, c).G_ - data_(r + 1, c + 1).G_) / 4)); }
					if (dN > dP) { X5 = saturate(((data_(r - 1, c + 1).R_ + data_(r + 1, c - 1).R_) / 2) + ((data_(r, c).G_ - data_(r - 1, c + 1).G_ + data_(r, c).G_ - data_(r + 1, c - 1).G_) / 4)); }
					if (dN == dP) { X5 = saturate(((data_(r - 1, c - 1).R_ + data_(r - 1, c + 1).R_ + data_(r + 1, c - 1).R_ + data_(r + 1, c + 1).R_) / 4) + ((data_(r, c).G_ - data_(r - 1, c - 1).G_ + data_(r, c).G_ - data_(r - 1, c + 1).G_ + data_(r, c).G_ - data_(r + 1, c - 1).G_ + data_(r, c).G_ - data_(r + 1, c + 1).G_) / 8)); }
					data_(r, c).R_ = X5;
				}
			}
		}
	}

	void write_on_file(std::ostream& os) {
		os << header.magicNumber << "\n";
		os << std::to_string(header.width) << " ";
		os << std::to_string(header.height) << " ";
		os << std::to_string(header.maxval) << "\n";
		for (size_t r = 0; r < data_.rows_; ++r) {
			for (size_t c = 0; c < data_.cols_; ++c) {
				os.write(reinterpret_cast<char*>(&data_(r, c)), sizeof(data_(r, c)));
			}
		}
	}

private: // Utils
	template <typename T>
	uint8_t saturate(T value) {
		if (value < 0) return 0;
		if (value > 255) return 255;
		return (uint8_t)value;
	}
};

int main(int argc, char** argv) {

	// Gestione della linea di comando
	if (argc != 3) {
		syntax();
	}
	std::string input_pgm_filename = argv[1];
	std::string output_prefix = argv[2];
	
	std::ifstream is(input_pgm_filename, std::ios::binary);
	if (!is) { error("Impossibile aprire il file " + input_pgm_filename); }

	// Lettura dell'immagine di input
	pgm<uint16_t> input16bpp(is);
	// Conversione in 8bpp e output
	pgm<uint8_t> output8bpp;
	input16bpp.pgm8bpp(output8bpp);
	std::ofstream os(output_prefix + "_gray.pgm", std::ios::binary);
	if (!os) { error("Impossibile aprire il file di output " + output_prefix + ".pgm"); }
	output8bpp.write_on_file(os);
	// Assegno ogni valore dei pixel al canale cromatico corretto
	ppm color_assigned;
	color_assigned.assingColor(output8bpp);
	std::ofstream os_color_assinged(output_prefix + "_bayer.ppm", std::ios::binary);
	if (!os_color_assinged) { error("Impossibile aprire il file di output " + output_prefix + "_bayer.ppm"); }
	color_assigned.write_on_file(os_color_assinged);
	// Bayer decode e interpolazione
	ppm color_output;
	color_output.readFromPPM(color_assigned, output_prefix); // Il file green e' stato creato fra la prima e la seconda passata
	std::ofstream os_final(output_prefix + "_interp.ppm", std::ios::binary);
	if (!os_final) { error("Impossibile aprire il file di output " + output_prefix + "_interp.ppm"); }
	color_output.write_on_file(os_final);

	return EXIT_SUCCESS;
}