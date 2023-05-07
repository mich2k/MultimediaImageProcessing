
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <iomanip>

void printTabs(uint8_t n) {
    for(uint8_t j = 0; j < n; j++)
        std::cout << "\t";
}


void bencoderec(std::stringstream& ss, std::stringstream* decoded, bool quote_flag, uint8_t tabs, bool* isPieces, bool* isError) {
    using namespace std;

    if (ss.fail() || ss.eof() || *isError) {
        return;
    }
    char c = 0;

    switch (ss.peek()) {
    case 'd':
    {
        ss.ignore();
        cout << "{\n";
        while (ss.peek() != 'e') {
            printTabs(tabs+1);
            bencoderec(ss, decoded, false, tabs + 1, isPieces, isError);
            cout << " => ";
            bencoderec(ss, decoded, true, tabs + 1, isPieces, isError);

        }
        ss.ignore();
        printTabs(tabs);
        cout << "}\n";

    }
        break;

    case 'l':
    {
        printTabs(tabs);
        cout << "[\n";
        printTabs(tabs);
        ss.ignore();
        char x = ss.peek();
        while (ss.peek() != 'e') {
            bencoderec(ss, decoded, false, tabs + 1, isPieces, isError);

        }
        ss.ignore();
        cout << "\n";
        printTabs(tabs);
        cout << "]\n";

        return;
    }

        break;

    case 'i':
    {
        int64_t val;
        ss.ignore(); // skippo la i
        ss >> val;
        cout << val << endl;
        ss.ignore(); // skippo la e
    }

        break;
    default:
    {
        size_t l;
        string str;
        ss >> l;
        ss.ignore(); // ignore :
        str.resize(l);
        ss.read(&str[0], l);
        for (auto& e : str)
            e = e < 32 ? '.' : e > 126 ? '.' : e;

        if (*isPieces) {    // se entro in questo if sono nel valore
            if (l % 20 != 0) {
                cout << endl << "ERROR PIECES HASH LEN!" << endl;
                *isError = true;
                break;
            }

            for (size_t k = 0; k < l; k++) {
                if (k % 20 == 0) {
                    cout << endl;
                    printTabs(tabs+2);
                }
                char x = str[k]; // uint8_t
                int y = static_cast<int>(x); // int 32 bit
                unsigned char z = static_cast<unsigned char>(y);
                int w = static_cast<int>(z);
                cout << setw(2) << w;
            }

            *isPieces = false; // resetto la flag
            break;
        }


        if (str == "pieces")    // stringa chiave trovata
            *isPieces = true;
        if(quote_flag)
            cout << '"' << str << '"' << endl;
        else
            cout << str;
        //bencoderec(ss, decoded, false, nest);

        //bencoderec(ss, decoded);
        //return;
    }

        break;
    }

}


int bencode(std::stringstream& ss) {
    std::stringstream decoded;

    bool flag = false;
    uint32_t tabs = 0;
    uint16_t nest = 0;
    bool isPieces = false, isError = false;

    bencoderec(ss, &decoded, false, 0, &isPieces, &isError);

    if (isError)
        return -1;

    return 0;

}



bool openFile(std::string filename, std::stringstream& ss) {
    using namespace std;
    ifstream in(filename, ios::binary);
    if (in.fail()) {
        return false;
    }
    noskipws(in);
    noskipws(ss);
    ss << in.rdbuf();
    in.close();

    return true;
}


int main(int argc, char** argv)
{
    using namespace std;

    string filename = "KickAss.torrent";
    stringstream ss(filename);

    if (!openFile(filename,ss))
        return -1;

    bencode(ss);

    return 0;
}

