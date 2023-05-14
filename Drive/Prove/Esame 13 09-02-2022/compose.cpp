#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void error(const std::string& errorMessage) {
	std::cout << "\n\tERRORE: " + errorMessage + "\n";
	exit(EXIT_FAILURE);
}

struct inFileOptions {
	std::string filename;
	size_t x = 0, y = 0;
};

struct header {
	std::string magicNumber;
	size_t height, width;
	uint8_t depth;
	size_t maxval;
	std::string tupltype;
};

struct pixel {
	uint8_t R_, G_, B_, A_;
	pixel() : R_(0), G_(0), B_(0), A_(0) {};
	pixel(uint8_t R, uint8_t G, uint8_t B, uint8_t A) : R_(R), G_(G), B_(B), A_(A) {};
};

struct PAMImage {

	header imageHeader;
	std::vector<pixel> imageData;

	pixel& operator()(size_t r, size_t c) { return imageData[r * imageHeader.width + c]; }

	void resize(size_t height, size_t width) { 
		imageHeader.height = height;
		imageHeader.width = width;
		imageData.resize(height * width);
	}

	void headerWriter(std::ostream& os) {
		os << "P7\n"
			<< "WIDTH " << std::to_string(imageHeader.width) << "\n"
			<< "HEIGHT " << std::to_string(imageHeader.height) << "\n"
			<< "DEPTH 4\n"
			<< "MAXVAL 255\n"
			<< "TUPLTYPE RGB_ALPHA\n"
			<< "ENDHDR\n";
	}

	char* rawdata() { return reinterpret_cast<char*>(imageData.data()); }
	int rawsize() { return imageHeader.height * imageHeader.width * sizeof(pixel); }

	void writeImage(std::ostream& os) {
		headerWriter(os);
		os.write(rawdata(), rawsize());
	}
};

bool headerReader(std::istream& is, header& headerIs) {

	is >> headerIs.magicNumber; // Leggo il magicNumber
	if (headerIs.magicNumber != "P7")
		return false;
	std::string label ,value; // Leggo le etichette e i valori
	is >> label; // WIDTH
	if (label != "WIDTH") { return false; }
	if (is.get() != ' ') { return false; };
	is >> value;
	headerIs.width = stoi(value);
	is >> label; // HEIGHT
	if (label != "HEIGHT") { return false; }
	if (is.get() != ' ') { return false; }
	is >> value;
	headerIs.height = stoi(value);
	is >> label; // DEPTH
	if (label != "DEPTH") { return false; }
	if (is.get() != ' ') { return false; }
	is >> value;
	headerIs.depth = stoi(value);
	is >> label; // MAXVAL
	if (label != "MAXVAL") { return false; }
	if (is.get() != ' ') { return false; }
	is >> value;
	headerIs.maxval = stoi(value);
	is >> label; // TUPLTYPE
	if (label != "TUPLTYPE") { return false; }
	if (is.get() != ' ') { return false; }
	is >> headerIs.tupltype;
	is >> label; // ENDHDR
	is.get();

	return true;
}

pixel alphaComposite(pixel pa, pixel pb) {
	pixel p0;
	p0.A_ = pa.A_ + pb.A_ * (1 - pa.A_);
	if (p0.A_ != 0) {
		p0.R_ = (pa.R_ * pa.A_ + (pb.R_ * pb.A_) * (1 - pa.A_)) / p0.A_;
		p0.G_ = (pa.G_ * pa.A_ + (pb.G_ * pb.A_) * (1 - pa.A_)) / p0.A_;
		p0.B_ = (pa.B_ * pa.A_ + (pb.B_ * pb.A_) * (1 - pa.A_)) / p0.A_;
	}
	else {
		p0.R_ = pb.R_;
		p0.G_ = pb.G_;
		p0.B_ = pb.B_;
	}

	return p0;
}

void pasteImage(PAMImage &dest, PAMImage &in, size_t x, size_t y) {
	for (size_t r = 0; r < in.imageHeader.height; ++r) {
		for (size_t c = 0; c < in.imageHeader.width; ++c) {
			size_t rDest = r + y;
			size_t cDest = c + x;

			if (rDest < dest.imageHeader.height && cDest < dest.imageHeader.width) {
				pixel p = alphaComposite(in(r, c), dest(rDest, cDest));
				dest(rDest, cDest) = p;
				// dest(rDest, cDest) = in(r, c);
			}
		}
	}
}

void readImageData(PAMImage& img, std::istream& is) {
	if (img.imageHeader.tupltype == "RGB") {
		uint8_t R, G, B;
		for (size_t r = 0; r < img.imageHeader.height; ++r) {
			for (size_t c = 0; c < img.imageHeader.width; ++c) {
				is.read(reinterpret_cast<char*>(&R), 1);
				is.read(reinterpret_cast<char*>(&G), 1);
				is.read(reinterpret_cast<char*>(&B), 1);
				pixel p(R, G, B, 255);
				img(r, c) = p;
			}
		}
	}
	else {
		for (size_t r = 0; r < img.imageHeader.height; ++r) {
			for (size_t c = 0; c < img.imageHeader.width; ++c) {
				is.read(reinterpret_cast<char*>(&img(r, c)), sizeof(pixel));
			}
		}
	}
}

void setAllPixelsTo(PAMImage& img, pixel p) {
	for (size_t r = 0; r < img.imageHeader.height; ++r) {
		for (size_t c = 0; c < img.imageHeader.width; ++c) {
			img(r, c) = p;
		}
	}
}

bool compose(PAMImage& out, std::istream& is, size_t x, size_t y) {

	PAMImage toBeCopied; // Immagine da copiare
	if (!headerReader(is, toBeCopied.imageHeader)) // Leggo l'header
		return false;
	toBeCopied.resize(toBeCopied.imageHeader.height, toBeCopied.imageHeader.width);
	readImageData(toBeCopied, is);
	pasteImage(out, toBeCopied, x, y);
	return true;
}

int main(int argc, char** argv) {

	// Linea di comando:
	std::vector<inFileOptions> inputFiles;
	std::string tmp;
	for (int i = 2; i < argc; ++i) {
		inFileOptions currentFile;
		std::string currentOption = argv[i];
		if (currentOption == "-p") {
			++i;
			tmp = argv[i];
			currentFile.x = stoi(tmp);
			++i;
			tmp = argv[i];
			currentFile.y = stoi(tmp);
			++i;
		}
		currentFile.filename = argv[i];
		inputFiles.push_back(currentFile);
	}

	// Apriamo il file di output:
	std::string outputFile = argv[1];
	outputFile.append(".pam");
	std::ofstream os(outputFile, std::ios::binary);
	if (!os) {
		error("Impossibile aprire il file di output " + outputFile);
	}

	// Calcolo la dimensione dell'immagine di output
	size_t maxW = 0, maxH = 0;
	for (size_t i = 1; i < inputFiles.size(); ++i) {
		inputFiles[i].filename.append(".pam");
		std::ifstream is(inputFiles[i].filename, std::ios::binary);
		if (!is) {
			error("Impossibile aprire il file di input " + inputFiles[i].filename);
		}
		header currentHeader;
		if (!headerReader(is, currentHeader)) {
			error("Impossibile leggere l'header del file " + inputFiles[i].filename);
		}
		if (currentHeader.height + inputFiles[i].y > maxH) { maxH = currentHeader.height + inputFiles[i].y; }
		if (currentHeader.width + inputFiles[i].x > maxW) { maxW = currentHeader.width + inputFiles[i].x; }
		is.close();
	}

	// 
	PAMImage out;
	out.imageHeader.height = maxH;
	out.imageHeader.width = maxW;
	out.resize(out.imageHeader.height, out.imageHeader.width);
	pixel p(0, 0, 0, 0);
	setAllPixelsTo(out, p);
	// Scriviamo il contenuto dell'immagine:
	

	// Compose
	for (size_t i = 1; i < inputFiles.size(); ++i) {
		std::ifstream is(inputFiles[i].filename, std::ios::binary);
		if (!is) {
			error("Impossibile aprire il file di input " + inputFiles[i].filename);
		}
		if (!compose(out, is, inputFiles[i].x, inputFiles[i].y)) {
			error("Problema durante la composizione del file di output e di " + inputFiles[i].filename);
		}
		is.close();
	}

	out.writeImage(os);

	return EXIT_SUCCESS;
}
