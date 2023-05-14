#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "mat.h"
#include "types.h"
#include "utils.h"

struct rate {
	size_t num, den;
};

struct streamHeader {

	size_t W = 0, H = 0;
	std::string C = "420jpeg";
	char I = 'p';
	rate F, A;
	std::string X;
};

struct y4m {

	streamHeader header;
	std::vector<mat<vec3b>> frames;

	bool readHeader(std::istream& is) {
		std::string field;
		is >> field;
		if (field != "YUV4MPEG2") { return false; }
		if (is.get() != ' ') { return false; }
		while (true) {
			char letterCode = is.get();
			is >> field;
			if (letterCode == 'W') { header.W = stoi(field); }
			if (letterCode == 'H') { header.H = stoi(field); }
			if (is.peek() == 0x0A) break;
			if (is.get() != ' ') return false;
		}
		std::getline(is, field, (char)0x0A);
		return true;
	}

	bool readFrames(std::istream& is) {
		std::string frameHeader;
		mat<vec3b> frame(header.H, header.W);
		mat<uint8_t> Y(header.H, header.W);
		mat<uint8_t> Cb(header.H / 2, header.W / 2);
		mat<uint8_t> Cr(header.H / 2, header.W / 2);
		while (std::getline(is, frameHeader, (char)0x0A)) {
			if (frameHeader.find("FRAME") == std::string::npos) break;
			// Leggo il contenuto del frame
			is.read(Y.rawdata(), Y.rawsize());
			is.read(Cb.rawdata(), Cb.rawsize());
			is.read(Cr.rawdata(), Cr.rawsize());
			for (int r = 0; r < frame.rows(); ++r) {
				for (int c = 0; c < frame.cols(); ++c) {
					double y = Y(r, c);
					double cb = upsample(Cb, r, c);
					double cr = upsample(Cr, r, c);
					saturate(y, cb, cr);
					frame(r, c) = YCbCrToRGB(y, cb, cr);
				}
			}
			frames.push_back(frame);
		}
		return true;
	}

// utilities
private:
	uint8_t upsample(mat<uint8_t>& img, size_t r, size_t c) {
		return img(r / 2, c / 2);
	}
	void saturate(double& y, double& cb, double& cr) {
		y = y < 16 ? 16 : y > 235 ? 235 : y;
		cb = cb < 16 ? 16 : cb > 240 ? 240 : cb;
		cr = cr < 16 ? 16 : cr > 240 ? 240 : cr;
	}
	vec3b YCbCrToRGB(double& y, double& cb, double& cr) {
		y -= 16;
		cb -= 128;
		cr -= 128;

		double R, G, B;

		R = 1.164 * y + 0.000 * cb + 1.596 * cr;
		G = 1.164 * y - 0.392 * cb - 0.813 * cr;
		B = 1.164 * y + 2.017 * cb + 0.000 * cr;

		R = R < 0 ? 0 : R > 255 ? 255 : R;
		G = G < 0 ? 0 : G > 255 ? 255 : G;
		B = B < 0 ? 0 : B > 255 ? 255 : B;

		vec3b RGB;
		RGB[0] = uint8_t(R);
		RGB[1] = uint8_t(G);
		RGB[2] = uint8_t(B);
		return RGB;
	}

};

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames){
	// Apro il file di input
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "\n\tImpossibile aprire il file di input " + filename + "\n";
		return false;
	}
	// Leggo il file y4m
	y4m img;
	if (!img.readHeader(is)) {
		std::cout << "\n\tFile " + filename + " non formattato correttamente.\n";
		return false;
	}
	if (!img.readFrames(is)) {
		std::cout << "\n\tFile " + filename + " non formattato correttamente.\n";
		return false;
	}
	frames = img.frames;
	return true;
}
