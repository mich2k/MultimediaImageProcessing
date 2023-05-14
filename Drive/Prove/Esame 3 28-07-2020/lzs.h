#ifndef LZS_H
#define LZS_H

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

void lzs_decompress(std::istream& is, std::ostream& os);
void lzs_compress(std::istream& is, std::ostream& os);

#endif // !LZS_H
