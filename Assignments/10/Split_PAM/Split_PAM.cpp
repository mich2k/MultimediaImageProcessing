// Split_PAM.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "mat.h"
#include <sstream>

using rgb = std::array<uint8_t, 3>;

// grayscale save pam
bool save_pam(const mat<uint8_t>& img, const std::string& filename)
{
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}
	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 1\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE GRAYSCALE\n";
	os << "ENDHDR\n";

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			os.put(img(r, c));
		}
	}

	//os.write(img.raw_data(), img.raw_size());

	return true;
}


// rgb load pam
bool load_pam(mat<rgb>& img, const std::string& filename)
{
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string magic_number;
	std::getline(is, magic_number);
	if (magic_number != "P7") {
		return false;
	}

	int w, h;
	while (1) {
		std::string line;
		std::getline(is, line);
		if (line == "ENDHDR") {
			break;
		}
		std::stringstream ss(line);
		std::string token;
		ss >> token;
		if (token == "WIDTH") {
			ss >> w;
		}
		else if (token == "HEIGHT") {
			ss >> h;
		}
		else if (token == "DEPTH") {
			int depth;
			ss >> depth;
			if (depth != 3) {
				return false;
			}
		}
		else if (token == "MAXVAL") {
			int maxval;
			ss >> maxval;
			if (maxval != 255) {
				return false;
			}
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype;
			ss >> tupltype;
			if (tupltype != "RGB") {
				return false;
			}
		}
		else {
			return false;
		}
	}

	img.resize(h, w);

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img(r, c)[0] = is.get();
			img(r, c)[1] = is.get();
			img(r, c)[2] = is.get();
		}
	}

	return true;
}

void splitPlane(mat<vec3b>& rgb_img, mat<uint8_t>& m, uint8_t i) {
    using namespace std;

	m.resize(rgb_img.rows(), rgb_img.cols());

    for (int r = 0; r < m.rows(); ++r) {
        for (int c = 0; c < m.cols(); ++c) {
            m(r, c) = rgb_img(r,c)[i];
        }
    }

    stringstream filename;

    char x = i == 0 ? 'R' : i == 1 ? 'G' : 'B';

    filename << "filename_" << x << ".pgm";

    save_pgm(filename.str(), m);

    return;
}

bool split(std::string filename) {
    using namespace std;

    mat<vec3b> in_img;
	bool ret = load_pam(in_img, filename);

    if (!ret)
        return false;

    vector<mat<uint8_t>> planes(3); // grayscale rgb planes

    for (uint8_t i = 0; i < 3; i++) {
        planes[i] = mat<uint8_t>();
        splitPlane(in_img, planes[i], i);
    }

    return true;
}



int main(int argc, char** argv)
{
    using namespace std;
	if (argc != 2)
		return 1;

    string filename(argv[1]);

    split(filename);

    return EXIT_SUCCESS;
}
