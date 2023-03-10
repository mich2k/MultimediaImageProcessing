// Assign-FIN-CPP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <vector>
#include <iostream>
#include <stdint.h>
#include <utility>
#include <inttypes.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

int main(int argc, char* argv[])
{
	using std::cout;
	using std::endl;
	using std::ofstream;
	using std::ifstream;
	using std::vector;

	if (argc != 3) {
		cout << "Usage: sort_int <filein.txt> <fileout.txt>\n";
		return 1;
	}

	
	ifstream input_stream(argv[1]);

	if (input_stream.fail()) {
		cout << "Error opening input file." << endl;
		return 1;
	}

	ofstream output_stream(argv[2]);

	if (output_stream) {
		cout << "Managed to open output file." << endl;
	}

	if (!output_stream) {
		cout << "Error opening output file." << endl;
		// is not needed to close streams since the
			// obj destructor ~ automatically destroys them
		// ifstream.close()
		return 1;
	}

	vector<double> v;


	while (true) {	// while(is >> num) { v.push_back(num); }
		double num;

		input_stream >> num;

		if (!input_stream) {
			break;
		}

		v.push_back(num);

		/*if (input_stream.good()) {	// good => boolean too
			v.push_back(num);
		}
		else if (input_stream.eof()) {
			break;
		}
		else {
			cout << "Warning: incorrect data input or data break.\n";
			break;
		}*/
	}

	std::sort(v.begin(), v.end()); // sort is a template


	for (size_t i = 0; i < v.size(); ++i) {
		// << returns a reference to itself
		// thanks to this is possible to chain operators
		// with multiple << operators
		output_stream << v[i] << endl;
		cout << v[i] << endl;
	}

	//-------------------------------------------------------//
	// versione con iteratori c++98, v1
	// 
		// iteratore: ogni iteratore ha classe specifica

	std::vector<double>::iterator it_start = v.begin();
	std::vector<double>::iterator it_stop = v.end();
	std::vector<double>::iterator it;

	// it++ chiama il next sull'iteratore
		// != non <=
	for (it = it_start; it != it_stop; ++it) {
		// * deferenzia e prende ciò che è puntato
		output_stream << *it << endl;
	}


	//-------------------------------------------------------//
	// versione con iteratori v2

	auto start = v.begin();
	auto stop = v.end();
	for (auto it = start; it != stop; ++it) {
		const auto& x = *it;
		output_stream << x << endl;
	}


	//-------------------------------------------------------//
	// range based for-loop

	// const se il ciclo non modifica vector
	for (/* const */ auto & x : v) { // anche `double x` andava
		output_stream << x << endl;
	}


	// ci permette di trattare uno stream come fosse un iteratore
		// l'assegnamento diventa <<
		// per ogni elemento viene copiato nell operatore
		// l'ultimo param è cio che va messo in append
		// ogni assegnamento produce <<
	std::copy(v.begin(), v.end(),
		std::ostream_iterator<double>(output_stream, '\n');


	// leggere v1 

	std::istream_iterator<double> start(input_stream);
	std::istream_iterator<double> stop; // costruttore default senza parametri

	for (auto it = start; it != stop; it++) {
		v.push_back(*it);
	}


	// leggere v2 con adapter
	

	// prende start, stop e copia i dati da start a stop
	// usando un back_inserter
	std::copy(start, stop, std::back_inserter(v));


	// leggere v3 con costruttore

	std::vector<double> iter_v(start, stop);



	return EXIT_SUCCESS;
}