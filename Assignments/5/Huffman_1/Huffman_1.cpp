#include <cstdint>
#include <iostream>
#include <iterator>
#include <vector>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <map>
#include <functional>
#include <iomanip>



template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}


template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitwriter {
	uint8_t buffer_;
	int n_ = 0;
	std::ostream& os_;

	std::ostream& write_bit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;
		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os) {}

	std::ostream& write(uint32_t u, uint8_t n) {
		//while (n --> 0) {
		//  write_bit(u >> n);
		//}
		for (int i = n - 1; i >= 0; --i) {
			write_bit(u >> i);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, uint8_t n) {
		return write(u, n);
	}

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			write_bit(bit);
		}
		return os_;
	}

	~bitwriter() {
		flush();
	}
};

class bitreader {
	uint8_t buffer_;
	uint8_t n_ = 0;
	std::istream& is_;

public:
	bitreader(std::istream& is) : is_(is) {}

	uint32_t read_bit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	uint32_t read(uint8_t n) {
		uint32_t u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return u;
	}

	uint32_t operator()(uint8_t n) {
		return read(n);
	}

	bool fail() const {
		return is_.fail();
	}

	explicit operator bool() const {
		return !fail();
	}
};


// huffman node
struct node {
	char sym_;
	float prob_;
	node* left_;
	node* right_;


	node(char sym, float prob, node* left, node* right) {
		sym_ = sym;
		prob_ = prob;
		left_ = left;
		right_ = right;
	}


	bool operator<(const node& other) const {
		return prob_ > other.prob_;
	}

};

// inizio funzione per stampare alberi binari

void printBT(const std::string& prefix, const node* node, bool isLeft)
{
	if (node != nullptr)
	{
		std::cout << prefix;

		std::cout << (isLeft ? "|--" : "L--");

		// print the value of the node
		std::cout << node->prob_ << std::endl;

		// enter the next tree level - left and right branch
		if (node->left_ != nullptr) {
			printBT(prefix + (isLeft ? "|   " : "    "), node->left_, true);
		}

		if (node->right_ != nullptr) {
			printBT(prefix + (isLeft ? "|   " : "    "), node->right_, false);
		}
	}
	else {
		return;
	}
}

void printBT(const node* node)
{
	printBT("", node, false);
}

// fine fun.



// init. huffman


bool nodePtrCompare(const node* a, const node* b) {
	return a->prob_ > b->prob_;					// devo definire l'attributo
												// del compare
}


/*
	Possibilità di ordinamento su oggetti custom:
		-> operare sugli oggetti, per fare ciò basta la funzione sort() con iteratore begin
			e iteratore end e definire/override dell'operatore > o < nella classe/struct
			dell'oggetto. (Implementato in questo esercizio ma non funzionante siccome uso un
			vettore di puntatori ad oggetti)

		-> operare sui puntatori ad oggetti, come in questo caso, il modo più efficace è definire una
			funzione compare, dati 2 puntatori const definiamo su che attributo operare, alternativamente
			si possono usare i functors
			

*/



int decode(const char* in_filename, const char* out_filename) {
	using namespace std;
	ifstream in(in_filename, ios::binary);
	ofstream out(out_filename);

	if (!in || !out) {
		cout << "Error opening stream/s" << endl;
		return 1;
	}

	string magic_word;

	bitreader br(in);

	while (magic_word.length() != 8) {
		char c = br.read(8);
		magic_word.push_back(c);
	}

	if (magic_word != "HUFFMAN1") {
		cout << "Wrong binary file, exiting.." << endl;
		return 1;
	}
	else {
		cout << "Right binary file" << endl;
	}


	uint8_t TableEntries;
	
	TableEntries = br.read(8);

	TableEntries = TableEntries == 0 ? 256 : TableEntries;

	cout << "Entries: " << (uint32_t)TableEntries << endl;;

	map<string, uint8_t> huffman_codes;

	for (uint8_t i = 0; i < TableEntries; ++i) {
		char sym = br.read(8);
		uint8_t len = br.read(5);
		string code = "";
		for (uint8_t j = 0; j < len; j++) {
			char bit = br.read_bit();
			code.push_back(bit + '0');
		}

		huffman_codes.insert({ code, sym });

	}

	uint32_t NumSymbols = br.read(32);
	vector<char> v;
	string r_code = "";

	for (uint32_t i = 0; i < NumSymbols;) {


		uint8_t bit = br.read_bit();
		r_code.push_back(bit + '0');
		if (huffman_codes.count(r_code) == 1) {
			// cout << (char)huffman_codes[r_code] << endl;
			out << huffman_codes[r_code];
			r_code = "";
			i++;
		}

	}


	return 0;
}


void assign_code( std::map<char, std::string>& codes, node* node,std::string& seq) {

	// a == 1101

	if (node) {

		if (node->sym_ != '\0') {
			// std::cout << node->sym_ << " " << seq << std::endl;
			codes[node->sym_] = seq;
		}


		seq.push_back('0');
		assign_code(codes, node->left_,seq );
		seq.pop_back();

		seq.push_back('1');
		assign_code(codes, node->right_, seq);
		seq.pop_back();
	}
}


int encode(const char* in_filename, const char* out_filename) {
	using namespace std;

	ifstream in(in_filename, ios::binary);
	ofstream out(out_filename, ios::binary);
	bitwriter bw(out);

	if (!in || !out) {
		return 1;
	}

	out << "HUFFMAN1";

	map<char, float> m;
	char c;
	while (in >> c) {

		m[c]++;
	}

	uint32_t sum = 0;
	
	for (auto it = m.begin(); it != m.end(); it++) {
		cout << it->first << " " << it->second << "; ";
		sum += it->second;
	}



	cout << endl;

	for (auto it = m.begin(); it != m.end(); it++) {
		it->second/=sum;
	}

	cout << endl << "Normalizzati:" << endl;

	for (auto it = m.begin(); it != m.end(); it++) {
		cout << it->first << " " << it->second << "; ";
	}

	if (m.size() == 256) {
		out << 0;
	}

	out << (uint8_t) m.size(); // n_items

	vector<node*> nodeptr_v;

	int i = 0;
	for (auto it = m.begin(); it != m.end(); it++) {
		node* t = new node{ it->first, it->second, nullptr, nullptr };
		nodeptr_v.push_back(t);
	}


	cout << endl << "Init. encoding" << endl;

	node* root;

	while (nodeptr_v.size() > 1) {
		sort(nodeptr_v.begin(), nodeptr_v.end(), nodePtrCompare);



		cout << endl << "v_size: " << nodeptr_v.size() << endl;

		for (auto& e : nodeptr_v) {
			cout << e->prob_ << endl;
		}



		node* l = nodeptr_v.back();
		nodeptr_v.pop_back();
		node* r = nodeptr_v.back();
		nodeptr_v.pop_back();

		node* parent = new node{ '\0', l->prob_ + r->prob_, l, r }; // \0 char dummy, in realtà non contiene simboli, risolvibile con l'inheritance ma direi di non complicare

		nodeptr_v.push_back(parent);


	}

	cout << endl << "End encoding" << endl;


	root = nodeptr_v.front();

	cout << endl << "Huffman Tree visual" << "\n\n";

	printBT(root);
	map <char, string> huff_codes;
	string seq = "";
	assign_code(huff_codes, root, seq);
	for (auto& it : huff_codes) {
		cout << it.first << " " << it.second << endl;


		bw.write(it.first, 8);
		uint32_t code_len = it.second.length();
		bw.write(code_len, 5);

		for (size_t i = 0; i < it.second.length(); i++) {
			uint8_t bit = it.second.c_str()[i] - '0';
			bw.write(bit , 1);
		}
	}

	bw.write(sum, 32);



	// in questo modo evito di caricare il file su un vettore interamente
	in.clear();
	in.seekg(0, in.beg);
	

	while (in >> c) {

		for (size_t i = 0; i < huff_codes[c].length(); i++) {
			uint8_t bit = huff_codes[c].c_str()[i] - '0';
			// cout << huff_codes[c].c_str()[i];
			bw.write(bit, 1);
		}
		cout << endl;

	}


	return 0;
}



int main(int argc, char** argv) {
	using namespace std;



	if (argc != 4) {
		cout << "usage huffman1 [c|d] <input file> <output file>" << endl;
		return 1;
	}

	if (*argv[1] != 'c' && *argv[1] != 'd') {
		cout << "usage huffman1 [c|d] <input file> <output file>" << endl;
		return 1;
	}

	int ret = *argv[1] == 'c' ? encode(argv[2], argv[3]) : decode(argv[2], argv[3]);

	if (ret != 0) {
		return 1;
	}

	return EXIT_SUCCESS;
}