#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

/*

	STEP:
		1.	introdurre template
		2.	definire oggetti: huffman, nodo, code
		3.	metodo create_table
		4.	metodo compute_lengths

*/


template <typename T>
struct huffman {

	struct node {
		T sym_;
		size_t prob_;
		node* l_= nullptr;
		node* r_= nullptr;


		node(T sym, size_t prob) : sym_(sym), prob_(prob) {};

		node(node* l, node* r) : prob_(l->prob_+r->prob_), sym_(0), l_(l), r_(r) {};

	};

	struct code {
		T sym_;
		uint32_t len_, val_;
	};

	vector<code> codes_table_;




	template <typename M>
	void create_table(M map) {
		vector <node*> v;

		for (auto& x : map) {
			node* n = new node(x.first, x.second);
			v.push_back(n);
		}

		sort(v.begin(), v.end(), [](node* a, node* b) {
			return a->prob_ > b->prob_;
			});


		while (v.size() > 1) {
			node* n1 = v.back();
			v.pop_back();
			node* n2 = v.back();
			v.pop_back();

			node* n = new node(n1, n2);


			auto it = lower_bound(v.begin(), v.end(), n, [](node* a, node* b) {
				return a->prob_ > b->prob_;
				});

			v.insert(it, n);
		}

		node* root = v.front();
		compute_lengths(root, 0);

		for (auto& e : codes_table_)
			cout <<  e.sym_ << " " << e.len_ << endl;
		


	}


	void compute_lengths(node* n, uint32_t len) {
		if (n->l_ == nullptr) {		// sappiamo di stare in una leaf siccome abbiamo init a nullptr nel costruttore
			T s = n->sym_;
			code c = { s, len, 0 };

			codes_table_.push_back(c);

		} else {
			compute_lengths(n->l_, len + 1);
			compute_lengths(n->r_, len + 1);
		}
		

	}



	void compute_canonical_codes() {
	
		uint32_t curr_len = 0, curr_val = 0;

		for (auto& e : codes_table_) {
			e.val_ = curr_val <<= (e.len_ - curr_len);	// shifto a sx la differenza di lunghezza

			/*
				sym	  len	val		|	len diff.
				0	  1		0		|		1-0
				2	  2		10		|		2-1 shift
				6	  3		110		|		3-2 shift
				7	  3		111		|		3-3 no shift
			*/

			curr_len = e.len_;

			curr_val++;

		}
	};


};