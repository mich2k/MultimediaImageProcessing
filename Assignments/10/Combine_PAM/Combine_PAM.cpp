#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <array>
#include <sstream>

// mat class
template<typename T>
class mat {
	int rows_ = 0, cols_ = 0;
	std::vector<T> data_;
public:
	mat() {}
	mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows* cols) {}

	int rows() const { return rows_; }
	int cols() const { return cols_; }
	int size() const { return rows_ * cols_; }

	T& operator()(int r, int c) { return data_[r * cols_ + c]; }
	const T& operator()(int r, int c) const { return data_[r * cols_ + c]; }

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	char* rawdata() { return reinterpret_cast<char*>(&data_[0]); }
	const char* rawdata() const { return reinterpret_cast<const char*>(&data_[0]); }
	int rawsize() const { return rows_ * cols_ * sizeof(T); }

	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }
};

using rgb = std::array<uint8_t, 3>;

// grayscale load pam
bool load_pam(mat<uint8_t>& img, const std::string& filename)
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
			if (depth != 1) {
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
			if (tupltype != "GRAYSCALE") {
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
			img(r, c) = is.get();
		}
	}

	return true;
}

// rgb save pam
bool save_pam(const mat<rgb>& img, const std::string& filename)
{
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}
	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 3\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE RGB\n";
	os << "ENDHDR\n";

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			os.put(img(r, c)[0]);
			os.put(img(r, c)[1]);
			os.put(img(r, c)[2]);
		}
	}

	//os.write(img.raw_data(), img.raw_size());

	return true;
}



std::string noExtensionFilename(std::string in) {
	size_t pos = in.rfind('_');
	std::string ret = in.substr(0, pos);
	return ret;
}

char colorPlaneFromFilename(std::string in) {
	size_t pos = in.rfind('.');
	return in.at(pos - 1);
}

bool combine(char** planes) {
	using namespace std;

	mat<uint8_t> R;
	mat<uint8_t> G;
	mat<uint8_t> B;

	for (uint8_t i = 0; i < 3; i++) {
		cout << planes[i] << endl;
		switch (colorPlaneFromFilename(planes[i])) {
		case 'R': {
			load_pam(R, planes[i]);
			break;
		}
		case 'G': {
			load_pam(G, planes[i]);
			break;
		}
		case 'B': {
			load_pam(B, planes[i]);
			break;
		}
		default: {
			cout << endl << "Wrong plane in at least one input filename" << endl;
			return false;
		}
		}
	}

	if (!(R.cols() == G.cols() && G.cols() == B.cols()))
		return false;
	if (!(R.rows() == G.rows() && G.rows() == B.rows()))
		return false;

	mat<rgb> m(R.rows(),R.cols());

	for (size_t r = 0; r < m.rows(); r++) {
		
		for (size_t c = 0; c < m.cols(); c++) {

			m(r, c)[0] = R(r, c);
			m(r, c)[1] = G(r, c);
			m(r, c)[2] = B(r, c);

		}

	}

	string noExtnoPlaneFilename = noExtensionFilename(planes[0]);

	stringstream outFilename;
	outFilename << noExtnoPlaneFilename << "_reconstructed.pam";

	save_pam(m, outFilename.str());
	return true;
}


int main(int argc, char** argv)
{
	using namespace std;
	if (argc != 4 && argc != 2) {
		cout << "usage: combine r.pam g.pam b.pam" << endl;
		return false;
	}
	char** planes;
	string s[3];

	if (argc == 4) {
		planes = argv + 1;
	}
	else {
		planes = (char**)malloc(sizeof(char**) * 3);
		stringstream e[3];
		e[0] << argv[1] << "_R.pam";
		e[1] << argv[1] << "_G.pam";
		e[2] << argv[1] << "_B.pam";

		for (uint8_t i = 0; i < 3; i++) {
			s[i] = e[i].str();
			planes[i] = &(s[i])[0];
		}

	}


	combine(planes);

	return EXIT_SUCCESS;
}