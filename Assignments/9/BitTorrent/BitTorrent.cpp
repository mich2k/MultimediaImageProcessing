// BitTorrent.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <fstream>
#include <deque>
#include <cstdint>
#include <string>
#include <iterator>
#include <cstdlib>

void insTabs(std::string* decoded, size_t n) {
    for (size_t t = 0; t < n; t++)
        decoded->push_back('\t');
    // decoded->insert(decoded->end(), '\t', n);
}

void bencode(std::deque<char>* to_decode, std::string* decoded, bool flag, uint32_t* tabs, bool quotes) {
    using namespace std;
    system("CLS");
    cout << *decoded << decoded->size();
    if (to_decode->size() == 0) {
        return;
    }

    //for (auto it : to_decode)
    //    cout << it;


    // caso integer
    if (to_decode->front() == 'i') {
        string ret;

        // rimuovo la 'i' iniziale
        to_decode->pop_front();

        while (to_decode->front() != 'e') {
            ret.push_back(to_decode->front());
            to_decode->pop_front();
        }
        // rimuovo la 'e' finale
        to_decode->pop_front();
        decoded->append(ret);
        if(!flag)
            bencode(to_decode, &ret, false, tabs,true);
        return;
    }

    // caso lista
    if (to_decode->front() == 'l') {
        //(*tabs)++;

        decoded->append("[\n");
        to_decode->pop_front();
        size_t cont = 0;
        while (to_decode->front() != 'e') {
            insTabs(decoded, *tabs);

            bencode(to_decode, decoded, true, tabs,true);
            if(to_decode->front() != 'e')
                decoded->append("\n");
            cont++;
        }
        decoded->append("\n");
        insTabs(decoded, (*tabs) - 1);
        decoded->append("]");
        // rimuovo le 'e' in coda per ogni nest trovato
        //for(size_t i = 0; i < cont; ++i)
        //    to_decode->pop_front();
        to_decode->pop_front();

        cout << "POP lista" << endl;
        bencode(to_decode, decoded, true, tabs,true);
        return;

    }


    // caso dictionary

    if (to_decode->front() == 'd') {
        (*tabs)++;
        to_decode->pop_front();
        decoded->append("{\n");

        while (to_decode->front() != 'e') {
            insTabs(decoded, *tabs);

            bencode(to_decode, decoded, true, tabs, false);
            decoded->append(" => ");
            bencode(to_decode, decoded, true, tabs, true);
            decoded->append("\n");
        }
        decoded->append("}");

    }


    // caso stringa
    if (to_decode->size() > 0 && to_decode->front() - '0' >= 0 && to_decode->front() - '0' <= 9) {
        string len, value;
        if (quotes)
            value.push_back('"');

        while (to_decode->front() != ':') {
            len.push_back(to_decode->front());
            to_decode->pop_front();
        }
        // rimuovo ':'
        to_decode->pop_front();
        value.append(to_decode->begin(), to_decode->begin() + stoi(len));
        if (quotes) {
            value.push_back('"');
        }

        to_decode->erase(to_decode->begin(), to_decode->begin() + stoi(len));

        decoded->append(value);
        if(!flag)
            bencode(to_decode, &value, false, tabs, true);
        return;
    }

    
}

bool readFile(std::string filename, std::deque<char>& t) {
    using namespace std;
    ifstream in(filename, ios::binary);
    noskipws(in);
    if (in.fail()) {
        return false;
    }

    istream_iterator<char> start(in);
    istream_iterator<char> stop;
    t.assign(start, stop);
    return true;
}


int _main()
{
    using namespace std;
    string in = "l3:byeli1ei2eee";
    //string in = "d3:cow3:moo4:spam4:eggse";
    string filename = "RacingTime.torrent";
    deque<char> t;

    if (!readFile(filename, t))
        return -1;

    for (auto e : t)
        cout << e;

    string decoded;
    //for (auto e : in) {
    //    t.push_back(e);
    //}



    bool flag = false;
    uint32_t tabs = 0;
    bencode(&t, &decoded, flag, &tabs, true);

    cout << decoded << endl;
}



/*

l3:byeli1ei2eee                 [ "bye", [ 1 2 ] ]
l -> inizio lista
3 -> len stringa successiva al ':'
e -> fine lista
qui c'è una lista dentro una lista
non c'è limite a quanti elementi ci siano dentro

*/

/*
d3:cow3:moo4:spam4:eggse

d -> dizionario
3 -> sicuramente c'è una stringa, è noto, è da 3 in questo caso
e -> indica fine dizionario

*/