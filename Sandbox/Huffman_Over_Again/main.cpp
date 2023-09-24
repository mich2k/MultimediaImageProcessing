
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <vector>
using namespace std;


// ACHTUNG: mistaking the raw_write & raw_read's reinterpret cast THROWS error C1001
	// Compiler error (NOT debuggable)
	// this error just means one among the two raws is wrong
		// common take from me:	the reinterpret cast
			// reinterpret_cast< (casting to?)> ( T* val )

template<typename T>
istream& raw_read(istream& in, T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}


template<typename T>
ostream& raw_write(ostream& out, T& v) {
	return out.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

template <typename T>
class bitreader {
private:
	uint8_t _buffer;
	size_t _n_bits;
	istream& _in;

	uint32_t read_bit() {
		if (_n_bits == 0) {	// I have an empty buffer :(
			_buffer = 0;
			raw_read(_in, _buffer);	// buffer filled 
			_n_bits = 8;			// buffer capacity restored
		}

		uint32_t bit = (_buffer >> (_n_bits - 1)) & 0b0000'0001;

		_n_bits--; // bit deduted from avialable _n_bits

		return bit;

	}



public:


	bitreader(istream& in) : _in(in), _n_bits(0) {};



	void read(T& v, size_t n) {	// reference to value i want to write in
										// how many bits i must read from istream
		while (n-- > 0) {
			uint32_t bit = read_bit();
			v = v | (bit << n);				// order preserving shift
		}
	}

};


template <typename T>
class bitwriter {
private:
	uint8_t _buffer;
	size_t _n_bits;
	ostream& _os;
public:
	bitwriter(ostream& os) : _n_bits(0), _buffer(0), _os(os) {};

	void write_bit(uint32_t bit) {

		_buffer <<= 1;	// free a slot
		_buffer |= bit;	// fill the slot
		_n_bits++;

		if (_n_bits == 8) {
			raw_write(_os, _buffer);
			_n_bits = 0;
			_buffer = 0;
		}

	}

	void write(const T& v, size_t n){

		while (n-- > 0) {
			uint32_t bit = (v >> n) & 1;
			write_bit(bit);
		}
	
	}

	~bitwriter() {	// flush condition
		while (_n_bits > 0)
			write_bit(0);		// zero padding
	}


};


// huffman class takes T templates
template <typename T>
class huffman {
private:

public:

	huffman() {};
	
	struct code {
		T _sym;
		uint16_t _len;
		uint16_t _code;
	};

	struct node {
		T _sym;
		size_t _freq;
		node* _l = nullptr;
		node* _r = nullptr;

		node(T sym, size_t freq) : _sym(sym), _freq(freq) {};
		node(node* l, node* r) {
			_sym = 0;	// parent has zero sym (my choice, fixable by inheritance ma chi c'ha il tempo sincero ao)
			_l = l;
			_r = r;
			_freq = l->_freq + r->_freq;
		};
	};

	vector<code> table;


	template <typename mapT>
	void build_table(mapT& m) {

		// from uint8_t to node tree
		vector<node*> v;
		vector<node*> tree;
		for (auto& p : m) {
			node* n = new node(p.first, p.second);
			v.push_back(n);
		}
		
		for (auto& n: v) {
			auto it = lower_bound(tree.begin(), tree.end(), n, [](node* a, node* b) {
				return a->_freq > b->_freq;
				});

			tree.insert(it, n);
		}

		// now i build the tree

		while (tree.size() != 1) {
			// grabbing the last two records of the tree
			// with these i build a parent node up to the root one.
	
			node* first_lowest = tree.back();
			tree.pop_back();		// kicking away the first lowest :(

			node* second_lowest = tree.back();
			tree.pop_back();		// kicking away the second lowest :(

			node* p = new node(first_lowest, second_lowest);	// parent node

			// we must re-insert the parent in a order.

			auto it = lower_bound(tree.begin(), tree.end(), p, [](node* a, node* b) {
				return a->_freq > b->_freq;
				});

			tree.insert(it, p);
		}

		// once done we can go to compute lengths, since now we have our tree
		// we pass the last ma..node standing and the starting code with a 0-start fashion
		this->compute_lengths(tree.front(), 0);	// i just like this man
		
	}


	// Preorder tree traversal
	void compute_lengths(node* t, uint16_t len) {

		// the code is assigned in the leaf condition !
		if (!t->_l) {

			// 
			//cout << t->_sym << " " << t->_freq << " " << len <<  endl;

			code c = { t->_sym, len, 0 };
			table.push_back(c);

			return;

		}

		compute_lengths(t->_l, len + 1);
		compute_lengths(t->_r, len + 1);

	}

	void compute_canonical_codes() {

		uint32_t curr_code = 0;
		size_t last_len = 0;

		sort(table.begin(), table.end(), [](code a, code b) {
			if (a._len != b._len)
				return a._len < b._len;
			else
				return a._sym < b._sym; }
		);

		for (code& c : table) {
			curr_code = (curr_code << (c._len - last_len));
			last_len = c._len;
			c._code = curr_code;

			cout << c._sym << " " << c._len << " " << c._code << endl;


			curr_code++;

		}

	}


	void encode(istream& in, ofstream& out) {
		unordered_map<uint8_t, size_t> freqs;

		bitwriter<uint8_t> bw(out);
		bitwriter<uint32_t> bw32(out);

		in.clear();
		in.seekg(0);
		uint8_t c = 0;
		uint32_t NumSymbols = 0;

		while (raw_read(in, c)) {
			freqs[c]++;
		}

		//for (auto& e : freqs)
		//	NumSymbols += e.second;


		this->build_table(freqs);
		this->compute_canonical_codes();

		string mw = "HUFFMAN2";
		out.write(&mw[0], 8);
		uint8_t n_entries = table.size();

		if (table.size() == 256)
			n_entries = 0;
		

		raw_write(out, n_entries);

		for (code entry : table) {
			bw.write(entry._sym, 8);
			bw.write(entry._len, 5);
		}

		// BitWriter - Big Endian
		// RawWrite - Little Endian

		in.clear();
		in.seekg(0);
		while (raw_read(in, c)) {		// old beautiful O(n^2), imo a hashmap indexed by char d suite better than a vector (unordered_map<uint8_t, code>)
			for (auto& entry : table) {	
				if (entry._sym == c) {
					NumSymbols++;
				}
			}
		}
		

		// obv not working. bw8 & bw32 dont share the buffer hence this approach would split the stream, sad
		//bw32.write(NumSymbols, 32);

		// sparatemi
		uint8_t first_byte = 0;
		uint8_t second_byte = 0;
		uint8_t third_byte = 0;
		uint8_t fourth_byte = 0;


		first_byte = (NumSymbols >> 24) & 255;
		second_byte = (NumSymbols >> 16) & 255;
		third_byte = (NumSymbols >> 8) & 255;
		fourth_byte = NumSymbols & 255;

		bw.write(first_byte,8);
		bw.write(second_byte,8);
		bw.write(third_byte,8);
		bw.write(fourth_byte,8);



		in.clear();
		in.seekg(0);
		// wait! now we have 2*O(n^2), what an experience

		while (raw_read(in, c)) {
			for (auto& entry : table) {
				if (entry._sym == c) {

					cout << "";

					bw.write(entry._code, entry._len);
				}
			}
		}

	}


	bool decode(istream& in, ostream& out) {
		string mw = "        "; // HUFFMAN2 -> 8 chars as init
		in.read(&mw[0], 8);
		if (mw != "HUFFMAN2")
			return false;

		table.clear();		// stay safe

		bitreader<uint8_t> br(in);
		bitwriter<uint8_t> bw(out);

		uint8_t TableEntries = 0;
		br.read(TableEntries, 8);
		uint16_t actualTableEntries;

		actualTableEntries = TableEntries == 0 ? 256 : TableEntries;

		for (uint16_t t = 0; t < actualTableEntries; t++) {
			code c;
			uint8_t sym = 0;
			br.read(sym, 8);
			c._sym = sym;
			uint8_t len = 0;
			br.read(len, 5);
			c._len = len;
			table.push_back(c);	
		}

		uint32_t NumSymbols = 0;


		// I wondery why I just didnt read and shifted each byte
			// so ugly im down for a loop to achieve this
				// for reasons like these ill fail the exam
				// fatdog is my dog.
		for (int8_t fatdog = 3; fatdog >= 0; fatdog--) {
			uint8_t byte = 0;
			br.read(byte, 8);
			NumSymbols |= byte;
			NumSymbols <<= (8 * fatdog);
		}

		// O(NumSymbols * TableEntries * max_bit)

		compute_canonical_codes();

		for (uint32_t n = 0; n < NumSymbols; n++) {
			code c = { 0,0,0 };
			uint8_t bit = 0;
			bool symFound = false;
			while (!symFound) {

				br.read(bit, 1);
				c._code |= bit;
				bit = 0;

				for (auto t : table) {			// hashtable me please (or at least filer by len)
					if (t._code == c._code) {	// my heart says improve this, my brain says is good, my eyes break the tie
						c._sym = t._sym;
						symFound = true;		// wrapping this as a function would allow me to remove the boolean and switch it with a return ~~ but im almost over
						bw.write(c._sym, 8);
						break;
					}
				}

				c._code <<= 1;

			}

		}




		return true;
	}


};



int main(int argc, char** argv) {
	
	ifstream in(argv[2], ios::binary);
	ofstream out(argv[3], ios::binary);

	if (!in || !out)
		return 1;
	if (argc != 4)
		return 1;




	bitreader<uint8_t> br(in);
	bitwriter<uint8_t> bw(out);

	/*
	* messin around
	unordered_map<uint8_t, size_t> f;
	uint8_t c = 'a';
	uint8_t d = 'b';
	uint8_t e = 'c';
	uint8_t r = 128;

	bw.write(c,8);
	bw.write(d, 8);
	bw.write(e, 8);
	bw.write(r, 4);
	h.build_table(f);
	h.compute_canonical_codes();
	*/

	huffman<uint8_t> h;
	if(*argv[1] == 'c')
		h.encode(in, out);
	else {
		if (*argv[1] == 'd')
			h.decode(in, out);
		else {
			cout << "usage:\thuffman2 [c|d] <input file> <output file>" << endl;
			return 1;
		}
	}

	return 0;
}