#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <iterator>

void syntax()
{
    std::cout << "Sintassi del programma \"Packbits\"\n"
              << "packbits mode inputFilename outputFilename\n"
              << "mode: c per compressione, d per decompressione\n";
    exit(EXIT_FAILURE);
}

void error(const std::string &msg)
{
    std::cout << "ERRORE: " + msg + "\n";
    exit(EXIT_FAILURE);
}

void writeSingletons(std::ofstream &os, std::vector<uint8_t> singletons)
{
    uint8_t nSingletons = singletons.size() - 1;
    os.write(reinterpret_cast<char *>(&nSingletons), 1);
    for (auto &x : singletons)
    {
        os.write(reinterpret_cast<char *>(&x), 1);
    }
}

void writeSequenza(std::ofstream &os, uint8_t dato, uint8_t occorrenze)
{
    uint8_t daScrivere = 257 - occorrenze;
    os.write(reinterpret_cast<char *>(&daScrivere), 1);
    os.write(reinterpret_cast<char *>(&dato), 1);
}

bool appartieneAllaSequenzaPrecedente(std::vector<uint8_t> &v, size_t pos)
{

    if (pos == 0)
    {
        return false;
    }

    if (v[pos] == v[pos - 1])
    {
        return true;
    }

    return false;
}

bool appartieneAllaSequenzaSuccessiva(std::vector<uint8_t> &v, size_t pos)
{

    if (pos == (v.size() - 1))
    {
        return false;
    }

    if (v[pos] == v[pos + 1])
    {
        return true;
    }

    return false;
}

bool isSingoletto(std::vector<uint8_t> &v, size_t pos, bool seqInterrotta = false)
{
    if (seqInterrotta)
    {
        if (appartieneAllaSequenzaPrecedente(v, pos) && !appartieneAllaSequenzaSuccessiva(v, pos))
            return true;
    }
    // Controllo di non essere al primo elemento
    if (pos == 0)
    {
        if (v[pos] != v[pos + 1])
            return true;
    }

    // Controllo di non essere all'ultimo elemento
    if (pos == (v.size() - 1))
    {
        if (v[pos] != v[pos - 1])
            return true;
    }

    // Controllo normale:
    if (v[pos] != v[pos - 1] && v[pos] != v[pos + 1])
    {
        return true;
    }

    return false;
}

bool isInizioSequenza(std::vector<uint8_t> &v, size_t pos)
{

    if (pos == v.size() - 1)
        return false;

    if (pos == 0)
    {
        if (v[pos] == v[pos + 1])
            return true;
    }

    if (v[pos] == v[pos + 1] && v[pos] != v[pos - 1])
    {
        return true;
    }

    return false;
}

bool isFineSequenza(std::vector<uint8_t> &v, size_t pos)
{

    if (pos == 0)
    {
        return false;
    }

    if (pos == (v.size() - 1))
    {
        if (v[pos] == v[pos - 1])
            return true;
    }

    if (v[pos] != v[pos + 1] && v[pos] == v[pos - 1])
    {
        return true;
    }

    return false;
}

bool isInSequenza(std::vector<uint8_t> &v, size_t pos)
{

    if (appartieneAllaSequenzaPrecedente(v, pos) || appartieneAllaSequenzaSuccessiva(v, pos) || isInizioSequenza(v, pos) || isFineSequenza(v, pos))
        return true;
    else
        return false;
}

void compress(std::ifstream &is, std::ofstream &os)
{
    std::vector<uint8_t> v{
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()};

    uint8_t occorrenze = 0;
    std::vector<uint8_t> singoletti;
    // Packbits inefficiente: perdoname por mi vida loca
    for (size_t i = 0; i < v.size(); ++i)
    {
        // Overflow check
        if (occorrenze == 128)
        {
            // Scrivo la sequenza sul file e azzero le occorrenze
            writeSequenza(os, v[i], occorrenze);
            occorrenze = 0;

            // Se il carattere corrente Ã¨ un singoletto (uguale ai precedenti, ma tale per via dell'overflow), lo inserisco nel vettore
            if (isSingoletto(v, i, true))
            {
                singoletti.push_back(v[i]);
            }
            else if (isInSequenza(v, i))
            {
                ++occorrenze;
            }
        }
        else
        {
            if (isInizioSequenza(v, i))
            {
                // Controllo il vettore dei singoletti
                if (!singoletti.empty())
                {
                    writeSingletons(os, singoletti);
                    singoletti.clear();
                }
            }

            if (isInSequenza(v, i))
            {
                ++occorrenze;
            }

            if (isFineSequenza(v, i))
            {
                writeSequenza(os, v[i], occorrenze);
                occorrenze = 0;
            }

            if (isSingoletto(v, i))
            {
                singoletti.push_back(v[i]);
                if (singoletti.size() == 128)
                {
                    writeSingletons(os, singoletti);
                    singoletti.clear();
                }
            }
        }
    }

    if(!singoletti.empty()){
        writeSingletons(os, singoletti);
        singoletti.clear();
    }
}

void decompress(std::ifstream &is, std::ofstream &os)
{
    std::vector<uint8_t> v{
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()};

    std::vector<uint8_t> singletons;
    uint8_t occorrenze;
    uint8_t corrente;
    // Itero fino a v.size() - 1 in modo tale da saltare il marcatore finale 128 dei file compressi con il packbits (80 in esadecimale)
    for (size_t i = 0; i < v.size() - 1;)
    {
        // Se leggo una sequenza
        if(v[i] > 127){
            // Memorizzo il valore delle occorrenze
            occorrenze = 257 - v[i];
            // Vado avanti di un elemento
            ++i;
            // Leggo il carattere da scrivere
            corrente = v[i];
            // Lo riporto una volta per ogni occorrenza
            for(uint8_t j = 0; j < occorrenze; ++j){
                os.write(reinterpret_cast<char*>(&corrente), 1);
            }
            // Passo all'elemento successivo
            ++i;
        }
        
        // Se devo leggere un grupppo di singoletti
        if(v[i] <= 127){
            // Leggo il numero di singoletti da riportare
            occorrenze = v[i] + 1;
            // Passo all'elemento successivo
            ++i;
            // Scorro i singoletti e li riporto sul file
            for(uint8_t j = 0; j < occorrenze; ++j){
                os.write(reinterpret_cast<char*>(&v[i]), 1);
                ++i;
            }
        }

    }
}

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        syntax();
    }

    std::string mode = argv[1];
    std::string inputFilename = argv[2];
    std::string outputFilename = argv[3];

    std::ifstream is(inputFilename, std::ios::binary);
    if (!is)
    {
        error("Impossibile aprire il file di input " + inputFilename);
    }

    std::ofstream os(outputFilename, std::ios::binary);
    if (!os)
    {
        error("Impossibile aprire il file di output " + outputFilename);
    }

    if (mode == "c")
    {
        std::cout << "Ricevuto comando per la compressione.\n";
        compress(is, os);
        uint8_t marcatore = 128;
        os.write(reinterpret_cast<char *>(&marcatore), 1);
    }
    else if (mode == "d")
    {
        decompress(is, os);
    }
    else
    {
        syntax();
    }

    return EXIT_SUCCESS;
}
