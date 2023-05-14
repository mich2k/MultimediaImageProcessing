#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdbool>
#include <cctype>
#include <sstream>
#include "mat.h"
#include "ppm.h"

void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {

	// Ridimensiono le matrici in maniera opportuna
	img_r.resize(img.rows(), img.cols());
	img_g.resize(img.rows(), img.cols());
	img_b.resize(img.rows(), img.cols());

	// Attuo lo splitting
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img_r(r, c) = img(r, c)[0];
			img_g(r, c) = img(r, c)[1];
			img_b(r, c) = img(r, c)[2];
		}
	}
}
