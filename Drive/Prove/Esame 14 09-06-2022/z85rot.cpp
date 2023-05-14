#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>

/*
	Il programma funziona correttamente, l'unica nota dolente riguarda il tempo di esecuzione. Per
	processare il file landscape.ppm (con i parametri c 13 e d 13), occorre settare la modalità
	release x64.
																							~ E.
*/

void syntax() {
	std::cout << "\n\tSintassi del programma Z85rot"
		<< "\n\tZ85rot {c | d} <input file> <output file>\n\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& errorMessage) {
	std::cout << "\n\tERRORE: " + errorMessage + ".\n";
	exit(EXIT_FAILURE);
}

struct PPMHeader {
	std::string magicNumber;
	size_t width, height;
	uint16_t maxval;
};

struct pixel {
	uint16_t R, G, B;
};

struct PPMImage {

	PPMHeader header;
	std::vector<uint16_t> imageData;

	PPMImage() {} // Empty constructor

	PPMImage(std::istream& is) { // Constructor
		readHeader(is); // Lettura dell'header
		readData(is); // Lettura del raster
	}

	void flushComment(std::istream& is) {
		if (is.peek() == '#') { // Se ho un commento, lo leggo e lo ignoro
			std::string comment;
			std::getline(is, comment, '\n');
		}
	}

	void readHeader(std::istream& is) {
		flushComment(is);
		is >> header.magicNumber;
		if (header.magicNumber != "P6") { error("Formato dell'immagine errato"); }
		if(!isspace(is.get())) { error("Formato dell'immagine errato"); }
		std::string stringValue;
		flushComment(is);
		is >> stringValue;
		header.width = stoi(stringValue);
		if (!isspace(is.get())) { error("Formato dell'immagine errato"); }
		flushComment(is);
		is >> stringValue;
		header.height = stoi(stringValue);
		if (!isspace(is.get())) { error("Formato dell'immagine errato"); }
		flushComment(is);
		is >> stringValue;
		header.maxval = stoi(stringValue);
		if (!isspace(is.get())) { error("Formato dell'immagine errato"); }
		flushComment(is);
	}

	void readData(std::istream& is) {
		if (header.maxval < 256) { // Se maxval è minore di 256, ogni sample occupa 1 byte
			for (size_t r = 0; r < header.height; ++r) {
				for (size_t c = 0; c < header.width; ++c) {
					uint8_t R, G, B;
					is.read(reinterpret_cast<char*>(&R), 1);
					is.read(reinterpret_cast<char*>(&G), 1);
					is.read(reinterpret_cast<char*>(&B), 1);
					imageData.push_back(R);
					imageData.push_back(G);
					imageData.push_back(B);
				}
			}
		}
		else { // Altrimenti ne occupa due
			for (size_t r = 0; r < header.height; ++r) {
				for (size_t c = 0; c < header.width; ++c) {
					pixel p;
					is.read(reinterpret_cast<char*>(&p.R), 2);
					is.read(reinterpret_cast<char*>(&p.G), 2);
					is.read(reinterpret_cast<char*>(&p.B), 2);
					pixelLEtoBE(p); // Converto il pixel da little endian a big endian
					imageData.push_back(p.R);
					imageData.push_back(p.G);
					imageData.push_back(p.B);
				}
			}
		}
	}

	void pixelLEtoBE(pixel& p) {
		uint16_t beR, beG, beB;
		beR = (p.R & 0b1111111100000000) >> 8;
		beR += (p.R & 0b0000000011111111) << 8;
		beG = (p.G & 0b1111111100000000) >> 8;
		beG += (p.G & 0b0000000011111111) << 8;
		beB = (p.B & 0b1111111100000000) >> 8;
		beB += (p.B & 0b0000000011111111) << 8;
		p.R = beR;
		p.G = beG;
		p.B = beB;
	}

	char z85Conversion(int val, int N = 0) {
		char converted = 85;
		val = val - N;
		// Rotation
		while (val < 0) { val += 85; }
		while (val >= 85) { val -= 85; }
		// Table
		if (val >= 0 && val <= 9) { converted = val + 48; }
		if (val >= 10 && val <= 35) { converted = val + 87; }
		if (val >= 36 && val <= 61) { converted = val + 29; }
		if (val == 62) { converted = '.'; }
		if (val == 63) { converted = '-'; }
		if (val == 64) { converted = ':'; }
		if (val == 65) { converted = '+'; }
		if (val == 66) { converted = '='; }
		if (val == 67) { converted = '^'; }
		if (val == 68) { converted = '!'; }
		if (val == 69) { converted = '/'; }
		if (val == 70) { converted = '*'; }
		if (val == 71) { converted = '?'; }
		if (val == 72) { converted = '&'; }
		if (val == 73) { converted = '<'; }
		if (val == 74) { converted = '>'; }
		if (val == 75) { converted = '('; }
		if (val == 76) { converted = ')'; }
		if (val == 77) { converted = '['; }
		if (val == 78) { converted = ']'; }
		if (val == 79) { converted = '{'; }
		if (val == 80) { converted = '}'; }
		if (val == 81) { converted = '@'; }
		if (val == 82) { converted = '%'; }
		if (val == 83) { converted = '$'; }
		if (val == 84) { converted = '#'; }
		return converted;
	}

	uint8_t z85decode(int val, int N = 0) {
		uint8_t decoded = 0;
		// Table
		if (val >= '0' && val <= '9') { decoded = val - 48; }
		if (val >= 'a' && val <= 'z') { decoded = val - 87; }
		if (val >= 'A' && val <= 'Z') { decoded = val - 29; }
		if (val == '.') { decoded = 62; }
		if (val == '-') { decoded = 63; }
		if (val == ':') { decoded = 64; }
		if (val == '+') { decoded = 65; }
		if (val == '=') { decoded = 66; }
		if (val == '^') { decoded = 67; }
		if (val == '!') { decoded = 68; }
		if (val == '/') { decoded = 69; }
		if (val == '*') { decoded = 70; }
		if (val == '?') { decoded = 71; }
		if (val == '&') { decoded = 72; }
		if (val == '<') { decoded = 73; }
		if (val == '>') { decoded = 74; }
		if (val == '(') { decoded = 75; }
		if (val == ')') { decoded = 76; }
		if (val == '[') { decoded = 77; }
		if (val == ']') { decoded = 78; }
		if (val == '{') { decoded = 79; }
		if (val == '}') { decoded = 80; }
		if (val == '@') { decoded = 81; }
		if (val == '%') { decoded = 82; }
		if (val == '$') { decoded = 83; }
		if (val == '#') { decoded = 84; }
		// Derotation
		val = decoded + N;
		while (val < 0) { val += 85; }
		while (val >= 85) { val -= 85; }
		decoded = val;
		return decoded;
	}

	std::vector<char> getEncodedData(int N = 0) {
		std::vector<char> encodedData;
		size_t absoluteIndex = 0;
		for (size_t i = 0; i < imageData.size(); i += 4) { // Ogni quattro valori leggo una cifra
			uint32_t valueToBeEncoded = imageData[i] << 24;
			valueToBeEncoded += imageData[i + 1] << 16;
			valueToBeEncoded += imageData[i + 2] << 8;
			valueToBeEncoded += imageData[i + 3];
			char sequence[5];
			for (size_t n = 0; n < 5; ++n) { // Ottengo i 5 valori mediante la conversione di base
				uint8_t resto = valueToBeEncoded % 85;
				sequence[4 - n] = resto;
				valueToBeEncoded = valueToBeEncoded / 85;
			}
			for (int n = 0; n < 5; ++n) { // Li riporto nel vettore in ordine inverso
				encodedData.push_back(z85Conversion(sequence[n], N * absoluteIndex));
				++absoluteIndex;
			}
		}
		return encodedData;
	}

	void writeCompressedStream(std::ostream& os, int N = 0) {
		// Riporto i dati di width e height
		os << std::to_string(header.width) << "," << std::to_string(header.height) << ",";
		// Il numero di byte da codificare deve essere un multiplo di 4, finché non lo è --> zero padding
		while (imageData.size() % 4 != 0) { imageData.push_back(0); }
		// Codifico i dati:
		std::vector<char> encodedData = getEncodedData(N);
		os.write(encodedData.data(), encodedData.size());
	}

	void readCompressedHeader(std::istream& is) {
		std::string stringVal;
		std::getline(is, stringVal, ','); // width
		header.width = stoi(stringVal);
		std::getline(is, stringVal, ','); // height
		header.height = stoi(stringVal);
		// Setto gli altri parametri
		header.magicNumber = "P6";
		header.maxval = 255;
	}

	void writePPM(std::ostream& os) {
		// header
		os << header.magicNumber;
		os << '\n';
		os << std::to_string(header.width) << ' ';
		os << std::to_string(header.height) << '\n';
		os << std::to_string(header.maxval) << '\n';
		// image data
		for (auto& byte : imageData) {
			os.write(reinterpret_cast<char*>(&byte), 1);
		}
	}

	void readCompressedImageData(std::istream& is, std::vector<uint8_t> &compressedStream) {
		uint8_t compressedVal;
		std::vector<uint8_t> compressedData;
		while (is.read(reinterpret_cast<char*>(&compressedVal), 1)) {
			compressedStream.push_back(compressedVal);
		}
	}

	uint32_t BEtoLE(uint32_t BEval) {
		uint32_t LEval = 0;
		LEval = BEval << 24; // L'ultimo byte diventa il primo
		LEval += (BEval & 0b00000000000000001111111100000000) << 8;
		LEval += (BEval & 0b00000000111111110000000000000000) >> 8;
		LEval += (BEval & 0b11111111000000000000000000000000) >> 24;
		return LEval;
	}

	void decompressImageData(std::vector<uint8_t>& compressedStream, int N = 0) {
		size_t absoluteIndex = 0;
		while (absoluteIndex < compressedStream.size()) {
			char sequence[5];
			for (size_t n = 0; n < 5; ++n) {
				sequence[n] = z85decode(compressedStream[absoluteIndex], N * absoluteIndex);
				++absoluteIndex;
			}
			uint32_t reconstructedVal = 0;
			for (int n = 0; n < 5; ++n) {
				reconstructedVal += sequence[n];
				if(n != 4)
					reconstructedVal *= 85;
			}
			uint32_t decompressedVal = BEtoLE(reconstructedVal);
			std::stringstream stream;
			stream.write(reinterpret_cast<char*>(&decompressedVal), 4);
			for (size_t n = 0; n < 4; ++n) {
				uint8_t byte;
				stream.read(reinterpret_cast<char*>(&byte), 1);
				imageData.push_back(byte);
			}
		}
		// Dopo aver effettuato la decompressione, rimuovo eventuali zero di padding
		imageData.resize(header.width * header.height * 3);
	}
};

void encode(std::istream& is, std::ostream& os, int N = 0) {
	// Apro l'immagine PPM ricevuta in input
	PPMImage img(is); // Costruzione dell'immagine in memoria: lettura dell'header e del raster
	img.writeCompressedStream(os, N);
}

void decode(std::istream& is, std::ostream& os, int N = 0) {
	// Apro l'input compresso
	PPMImage img;
	img.readCompressedHeader(is);
	std::vector<uint8_t> compressedStream;
	img.readCompressedImageData(is, compressedStream);
	img.decompressImageData(compressedStream, N);
	img.writePPM(os);
}

int main(int argc, char** argv) {

	// Command line management
	if (argc != 5) {
		syntax();
	}
	std::string option = argv[1];
	std::string N = argv[2];
	std::string inputFilename = argv[3];
	std::string outputFilename = argv[4];
	// Apertura dei file:
	std::ifstream is(inputFilename, std::ios::binary);
	if (!is) { error("Impossibile aprire il file di input " + inputFilename); }
	std::ofstream os(outputFilename, std::ios::binary);
	if (!os) { error("Impossibile aprire il file di input " + outputFilename); }
	// Option check
	if (option != "c" && option != "d") { syntax(); }
	if (option == "c") { encode(is, os, stoi(N)); }
	if (option == "d") { decode(is, os, stoi(N)); }

	return EXIT_SUCCESS;
}

