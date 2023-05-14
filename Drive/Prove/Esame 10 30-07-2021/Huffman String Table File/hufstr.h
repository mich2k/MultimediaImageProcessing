#ifndef HUFSTR_H
#define HUFST_H
#include <fstream>
#include <iterator>
#include <unordered_map>
#include <memory>
#include <vector>
#include <queue>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>

struct elem {
	uint8_t _len = 0;
	uint32_t _code = 0;
};

struct triplet {
	uint32_t _sym, _len, _code;
};

class hufstr {
public:
	hufstr();
	std::vector<triplet> _huff_table;
	std::unordered_map<uint8_t, elem> _table;
	uint8_t getSym(uint32_t code) const;
	elem getEntry(uint32_t code) const;
    std::vector<uint8_t> compress(const std::string& s) const;
    std::string decompress(const std::vector<uint8_t>& v) const;
};

#endif
