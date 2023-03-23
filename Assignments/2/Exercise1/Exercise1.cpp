#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <map>
#include <utility>
#include <iostream>
#include <fstream>

#include <iomanip>
// pointless function that i will keep in order to exercise with references, hashmaps and references
    // useless for the scope of the exercise

int countOccurrences(std::map<uint8_t,size_t> &myMap, std::ofstream& out) {


    for (auto it_i = myMap.begin(); it_i != myMap.end(); ++it_i) {
        uint8_t curr_char = it_i->first;

        for (auto it_j = myMap.begin(); it_j != myMap.end(); ++it_j) {
            if (it_j->first == it_i->first) {
                it_i->second++;
            }
        }
    }


    return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {
    using std::cout;
    using std::endl;
    using std::ifstream;
    using std::ofstream;

    std::map<uint8_t, size_t> kv_map;

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
    ofstream out_stream(argv[2]);
    if (out_stream.fail()) {
        cout << "Error: output file stream" << endl;
        return EXIT_FAILURE;
    }


    // reading

    for (size_t i = 0; ; ++i) {

        // first thing read from stream

        uint8_t c;
        in_stream >> std::noskipws >> c; // NO skip whitespace manipulator


        // then check for stream errors or eof reached

        if (in_stream.eof()) {
            break;
        }

        if (in_stream.fail()) {
            cout << "Error while reading" << endl;
            return EXIT_FAILURE;
        }

        // adding uint8_t to map keys, then incrementing counter
        kv_map[c]++;
        //kv_map.insert(std::make_pair(c, 0));

    }
    

    // for-each print
    for (const auto& kv : kv_map)
    {
        using namespace std;
        /*
            setBase(16) base 16
            setw(2) width
            setfill(0) use 0 instead of spaces
        
        */
        cout << uppercase <<  hex << setw(2) << setfill('0') << (int)kv.first << '\t' << dec << kv.second << endl;

        out_stream << uppercase << hex << setw(2) << setfill('0') << (int)kv.first << '\t' << dec << kv.second << endl;
    }


    return EXIT_SUCCESS;
}