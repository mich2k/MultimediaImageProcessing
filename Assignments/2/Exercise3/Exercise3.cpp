#include <iostream>
#include <cstdint> // cpp version of C library
#include <fstream>
#include <vector>
#include <iterator>


template <typename T>
std::istream& raw_read(std::istream& in,T& val, size_t size = sizeof(T)) {
    return in.read(reinterpret_cast<char*>(&val), size);
}


int teacherReadFromFile(std::ifstream& in, std::vector<int>& v, std::ofstream& out) {


    while(true) {

        int x;
        if (raw_read(in, x).eof()) {
            break;
        }

        std::cout << x << std::endl;

        v.push_back(x);
    }


    return EXIT_SUCCESS;
}

// get reads int 32 bit

int main(int argc, char* argv[])
{

    using std::cout;
    using std::ifstream;
    using std::ofstream;
    using std::endl;
    using std::vector;


    vector<int32_t> v;


    if (argc != 3) {
        cout << "Usage: frequencies <input_file> <output_file>" << endl;
        return EXIT_FAILURE;
    }


    // binary input file

    ifstream in_stream(argv[1], std::ios::binary);
    if (in_stream.fail()) {
        cout << "Error: input file stream" << endl;
        return EXIT_FAILURE;
    }


    // textual output file
    ofstream out_stream(argv[2]); // avoids CR+LF translation
    if (out_stream.fail()) {
        cout << "Error: output file stream" << endl;
        return EXIT_FAILURE;
    }

    // int ret = readFromFile(in_stream, v);

    int ret = teacherReadFromFile(in_stream, v, out_stream);

    if (ret != 0) {
        cout << "Error in readFromFile function." << endl;
        return EXIT_FAILURE;
    }

    /*for (const auto& elem : v) {
        cout << elem << endl;
    }*/


    return EXIT_SUCCESS;
}