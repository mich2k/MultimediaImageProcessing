#ifndef PBM_H
#define PBM_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <cctype>
#include <cmath>

struct BinaryImage {
    int W;
    int H;
    std::vector<uint8_t> ImageData;

    bool readHeader(std::istream& is);
    bool readDataRaster(std::istream& is);
    bool ReadFromPBM(const std::string& filename);
};

struct Image {
    int W;
    int H;
    std::vector<uint8_t> ImageData;
};

Image BinaryImageToImage(const BinaryImage& bimg);

#endif
