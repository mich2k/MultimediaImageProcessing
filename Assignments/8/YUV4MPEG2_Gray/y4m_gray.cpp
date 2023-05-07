// YUV4MPEG2_Gray.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "mat.h"
#include "pgm.h"
#include <sstream>
#include <iomanip>


bool readHeader(std::ifstream& in, std::map<uint8_t, std::string>& header) {
    
    std::string chroma = "420jpeg";
    std::string v;
    const char poss[] = {'W','H','C','I','F','A','X'};
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

bool readFrames(std::ifstream& in, std::map<uint8_t, std::string>& header, std::vector<mat<uint8_t>>& frames) {
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

        frames.push_back(Y);
    }

    return true;
}

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
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
    //std::string filename = "foreman_cif.y4m";

    std::vector<mat<uint8_t>> frames;
    if (y4m_extract_gray(filename, frames)) {
        for (size_t i = 0; i < frames.size(); ++i) {
            std::stringstream ss;
            ss << std::setfill('0');
            ss << "frame" << std::setw(3) << i << ".pgm";
            save_pgm(ss.str(), frames[i]);
        }
    }

    return 0;
}




/*

    C (chroma subsampling) = sequenza che dice come sono stati mem. i canali colore YCbCr; 420
        larghezza regione logica (usually: 4)
        numero campioni colore della prima riga di J pixels (quanti colori ci sono all interno della prima riga? Se è 42, 
            di 4 pixel ho 2 campioni colore; ogni 4 Y ho due di CbCr)
        nella seconda riga ci sono altri campioni? 


        Per ogni 4 pixel Y ci sono 1 di Cb e 1 di Cr

    I indica l'interlacciamento, il fotogramma è progressivo? Se è progressivo ho tutti i pixel dell'immagine; L'interlacciamento 
        manda prima tutte le righe dispari e poi tutte quelle pari per compensare la tv catodica (c'è prima la meta sup o inf)


    F è il frame rate a cui stiamo mandando le informazioni?

    A aspect ratio, importante siccome assumiamo sempre che i pixel siano quadrati ma non è detto

    X



    dopo FRAME ho W*H pixel/byte divisi in Y e CbCr e poi FRAME nuovamente per ogni frame
    dobbiamo ritornare tutti i fotogrammi Y

    ignoro frame rate ed aspect ratio
    solo progressive no interlacciamento
    dobbiamo solo caricarli nella mat


    pgm formato visualizzabile
    ppm formato a colori

*/




