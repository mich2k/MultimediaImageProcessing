#include <iostream>
#include <cstdint> // cpp version of C library
#include <fstream>
#include <vector>
#include <iterator>


template <typename T>
std::ostream& raw_write(std::ostream& out, const T& val, size_t size=sizeof(T)) {
    return out.write(reinterpret_cast<const char*>(&val), size);

}


int teacherReadFromFile(std::ifstream& in, std::vector<int>& v, std::ofstream& out) {



    std::istream_iterator<int32_t> is_start(in);
    std::istream_iterator<int32_t> is_stop;
    // if we would use temp vars at first we have to
        //  call the default constructor for is_stop as is_stop()
        //  and we would have an error thrown as "The Most Vexing Parse"
        //  the compiler does not read this as a constructor but as a
        //  function prototype, hence thinking that (in) is not a constructor
        //  BUT the first parameter of a function

        //  to avoid this uniform initilization has been introduced in c++11
        //  since can not be mistaken as a function def.
 
    v.assign(is_start, is_stop); // assign method accepts two iterators! we can use
                                // this after the vector initialization


    // EXAMPLE: HOW TO WRITE BINARY

    int32_t x = 266;

    /*
        
    */
    
    // out.write(reinterpret_cast<const char* >(&x), 4);
    // raw_write(out, x);

    for (const auto& e : v) {
        raw_write(out, e);
    }

    // thanks to c++ "Template deduction" I can avoid writing
        // raw_write<typename>(out,x);

    // EXAMPLE: CASTING IN CPP








    return EXIT_SUCCESS;
}


// no const ifstream if reading
int readFromFile(std::ifstream& in, std::vector<int>& v) {
    using std::cout;
    using std::endl;

    while (true) {


        if (in.eof()) {
            break;
        }
        if (in.fail()) {
            cout << "Error while reading" << endl;
            return EXIT_FAILURE;
        }

        int elem;

        in >> elem;

        v.push_back(elem);

    }



    return EXIT_SUCCESS;
}

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

    ifstream in_stream(argv[1]);
    if (in_stream.fail()) {
        cout << "Error: input file stream" << endl;
        return EXIT_FAILURE;
    }


    // textual output file
    ofstream out_stream(argv[2], std::ios::binary); // avoids CR+LF translation
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

    for (const auto& elem : v) {
        cout << elem << endl;
    }


    return EXIT_SUCCESS;
}