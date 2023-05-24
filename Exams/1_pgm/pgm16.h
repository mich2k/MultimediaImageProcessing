#include <iostream>
#include <fstream>
#include <string> // c'è getline() per prendere il commento
#include <cstdint>
#include "mat.h"
using namespace std;



#ifndef PGM16_H
#define PGM16_H


bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue);



#endif // PGM16_H