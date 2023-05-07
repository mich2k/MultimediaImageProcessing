// YUV4MPEG2_Gray.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "mat.h"
#include "ppm.h"
#include <sstream>
#include <iomanip>


bool readHeader(std::ifstream& in, std::map<uint8_t, std::string>& header) {

    std::string chroma = "420jpeg";
    std::string v;
    const char poss[] = { 'W','H','C','I','F','A','X' };
    header['C'] = "420jpeg";
    while (true) {
        bool flag = false;
        char next = in.get();
        if (next == '\n') {
            break;
        }
        if (next != ' ') {
            return false;
        }
        char tag = in.get();

        for (auto e : poss) {
            if (tag == e)
                flag = true;
        }

        if (!flag)
            return false;

        in >> v;
        header[tag] = v;

    }
    if (header.count('W') != 1 || header.count('H') != 1)
        return false;


    return true;
}

bool checkMagicWord(std::ifstream& in, std::string magicword) {

    std::string word;
    in >> word;
    return word == magicword;
}

bool readFrames(std::ifstream& in, std::map<uint8_t, std::string>& header, std::vector<mat<vec3b>>& frames) {
    using namespace std;
    string frame_magic_word;
    int H = stoi(header['H']);
    int W = stoi(header['W']);


    mat<uint8_t> Y(H, W);
    mat<uint8_t> Cb(H / 2, W / 2);
    mat<uint8_t> Cr(H / 2, W / 2);

    // ACHTUNG: in.read(&frame_magic_word[0], 5); for some reason does not work the if statement

    string interlacing, application;
    while (true) {
        if (in.fail())
            return false;

        if (in.eof())
            break;
        in >> frame_magic_word;
        if (frame_magic_word != "FRAME") {
            return false;
        }


        // ogni frame contiene X ed I di nuovo

        while (true) {
            char next = in.get();
            if (next == '\n') {
                break;
            }
            if (next != ' ') {
                return false;
            }
            char tag = in.get();
            switch (tag) {
            case 'I':
                in >> interlacing;
                break;
            case 'X':
                in >> application;
                break;
            default:
                return false;
            }


        }

        /*
        !   ACHTUNG: fs must be opened as binary
        !               here ios::binary was missing hence after the first frame
        !               the \n got translated and the input stream was failing
        !
        */


        //    // sono salvati in modo sequenziale
        // "Frames are made up of the Y values of all pixels, followed by all Cb values, 
        // followed by all Cr values. Cb and Cr are suitably subsampled."


        in.read(Y.rawdata(), Y.rawsize());
        
        if (in.fail())
            return false;
        in.read(Cb.rawdata(), Cb.rawsize());
        if (in.fail())
            return false;
        in.read(Cr.rawdata(), Cr.rawsize());
        if (in.fail())
            return false;

        mat<vec3b> rgb_frame(H,W);
        using i_vec3b = std::array<int, 3>;

        i_vec3b pix, pp_pix;
        for (int r = 0; r < rgb_frame.rows(); r++) {
            for (int c = 0; c < rgb_frame.cols(); c++) {
                pix[0] = Y(r, c);
                pix[1] = Cb(r / 2, c / 2);
                pix[2] = Cr(r / 2, c / 2);

                pix[0] = pix[0] < 16 ? 16 : pix[0]>235 ? 235 : pix[0];
                pix[1] = pix[1] < 16 ? 16 : pix[1]>240 ? 240 : pix[1];
                pix[2] = pix[2] < 16 ? 16 : pix[2]>240 ? 240 : pix[2];

                pix[0] -= 16;
                pix[1] -= 128;
                pix[2] -= 128;

                pp_pix[0] = (1164 * pix[0] + 1596 * pix[2]) / 1000;
                pp_pix[1] = (1164 * pix[0] - 392 * pix[1] - 813 * pix[2]) / 1000;
                pp_pix[2] = (1164 * pix[0] + 2017 * pix[1]) / 1000;



                pp_pix[0] = pp_pix[0] < 0 ? 0 : pp_pix[0]>255 ? 255 : pp_pix[0];
                pp_pix[1] = pp_pix[1] < 0 ? 0 : pp_pix[1]>255 ? 255 : pp_pix[1];
                pp_pix[2] = pp_pix[2] < 0 ? 0 : pp_pix[2]>255 ? 255 : pp_pix[2];





                vec3b rgb;
                for (int i = 0; i < 3; i++)
                    rgb[i] = static_cast<uint8_t>(pp_pix[i]);

                rgb_frame(r, c) = {rgb[0], rgb[1], rgb[2]};

                //pix[0] = Y(r, c);
                //pix[1] = Cb(r / 2, c / 2);
                //pix[2] = Cr(r / 2, c / 2);


                //// YCbCr capping

                //pix[0] = pix[0] < 16 ? 16 : pix[0] > 235 ? 235 : pix[0];
                //pix[1] = pix[1] < 16 ? 16 : pix[1] > 240 ? 240 : pix[1];
                //pix[2] = pix[2] < 16 ? 16 : pix[2] > 240 ? 240 : pix[2];


                //// adj

                //pix[0] -= 16;
                //pix[1] -= 128;
                //pix[2] -= 128;


              
                //// color space conversion

                //pp_pix[0] = 1164 * pix[0] + 1596 * pix[2];
                //pp_pix[1] = 1164 * pix[0] - 392 * pix[1] - 813 * pix[2];
                //pp_pix[2] = 1164 * pix[0] + 2017 * pix[1];


                //// RGB capping + casting

                //vec3b uint_rgb;


                //for (uint8_t i = 0; i < 3; i++) {
                //    pp_pix[i] /= 1000;
                //    uint_rgb[i] = static_cast<uint8_t>(pp_pix[i]);

                //    uint_rgb[i] = uint_rgb[i] < 0 ? 0 : uint_rgb[i] > 255 ? 255 : uint_rgb[i];

                //}

                //rgb_frame(r, c) = uint_rgb;
            }
        }

        frames.push_back(rgb_frame);
    }

    return true;
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
    using namespace std;

    ifstream in(filename, ios::binary);
    if (in.fail())
        return false;

    if (!checkMagicWord(in, "YUV4MPEG2")) {
        cout << endl << "Wrong binary file" << endl;
        return false;
    }

    map<uint8_t, string> header;
    if (!readHeader(in, header))
        return false;

    int H = stoi(header['H']);
    int W = stoi(header['W']);

    if (W < 0 || H < 0 || header['C'] != "420jpeg") {
        return false;
    }

    readFrames(in, header, frames);

    return true;
}


int main()
{
    std::string filename = "720p_stockholm_ter.y4m";

    std::vector<mat<vec3b>> colors;
    if (y4m_extract_color(filename, colors)) {
        for (size_t i = 0; i < colors.size(); ++i) {
            std::stringstream ss;
            ss << std::setfill('0');
            ss << "color" << std::setw(3) << i << ".ppm";
            save_ppm(ss.str(), colors[i]);
        }
    }

    return 0;
}

