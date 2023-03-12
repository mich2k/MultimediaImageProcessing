#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

// il move constructor deve essere sempre noexcept

struct mio_int {
    int val_;
    explicit mio_int(int val) : val_(val) {}

    explicit operator int() {
        return val_;
    }
};

int main(int argc, char* argv[])
{/*
    int x = 4;
    mio_int a(5);

    a = mio_int(x);
    x = int(a);*/

    if (argc != 3) {
        std::cout << "Usage: sort_int <filein.txt> <fileout.txt>\n";
        return 1;
    }

    std::ifstream is(argv[1]);
    if (!is) {
        std::cout << "Error opening input file.\n";
        return 1;
    }

    std::ofstream os(argv[2]);
    if (!os) {
        std::cout << "Error opening output file.\n";
        return 1;
    }

    std::istream_iterator<double> start(is);
    std::istream_iterator<double> stop;
    std::vector<double> v(start, stop);

    /*for (auto it = start; it != stop; ++it) {
        v.push_back(*it);
    }*/
    // std::copy(start, stop, std::back_inserter(v));



    /*double num;
    while (is >> num) {
        v.push_back(num);
    }*/

    std::sort(v.begin(), v.end());

    /* Versione classica (che va bene!)
    for (size_t i = 0; i < v.size(); ++i) {
        os << v[i] << '\n';
    }//*/
    /* Versione con iteratori 1
    std::vector<double>::iterator start = v.begin();
    std::vector<double>::iterator stop = v.end();
    std::vector<double>::iterator it;
    for (it = start; it != stop; ++it) {
        double x = *it;
        os << x << '\n';
    }//*/
    /* Versione con iteratori 2
    auto start = v.begin();
    auto stop = v.end();
    for (auto it = start; it != stop; ++it) {
        const auto& x = *it;
        os << x << '\n';
    }//*/
    /* Versione con range-based for loop
    for (const auto& x : v) {
        os << x << '\n';
    }//*/
    /* Versione con copy e ostream_iterator */
    std::copy(v.begin(), v.end(),
        std::ostream_iterator<double>(os, "\n"));
    //*/
    return 0;
}