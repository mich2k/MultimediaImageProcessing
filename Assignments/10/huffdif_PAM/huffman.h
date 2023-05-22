#pragma once
#include <algorithm>
#include <vector>
#include <cstdint>
#include <map>
#include <bitset>

template <typename T>
struct huffman {

	struct code {
		T sym_;
		uint32_t len_;
		uint32_t val_;
	};


	struct node {
		T sym_;
		uint32_t prob_;
		node* l_=NULL;
		node* r_=NULL;

		node(T sym, uint32_t prob) {
			sym_ = sym;
			prob_ = prob;
		}

		node(node* l, node* r) {
			sym_ = '\0';
			prob_ = l->prob_ + r->prob_;
			l_ = l;
			r_ = r;
		}

	};

	std::vector<code> codes_table_;


	template <typename mapT>
	void compute_codes_table(mapT& m){
		std::vector<node*> v;
		
		// creo il nodepointer vector
		for (auto& e : m) {
			node* n = new node(e.first, e.second);
			v.emplace_back(n);
		}

		// ordino in modo decrescente per occorrenze

		std::sort(v.begin(), v.end(), [](node* a, node* b) {
			return a->prob_ > b->prob_;
			});

		// creo l'albero con inserimento ordinato
		while (v.size() > 1) {
			node* f1 = v.back();
			v.pop_back();
			node* f2 = v.back();
			v.pop_back();
			node* p = new node(f1,f2); // parent



			auto it = std::lower_bound(v.begin(), v.end(), p, [](node* a, node* b) {
				return a->prob_ > b->prob_;
				}
			);

			v.insert(it, p);
		}


		node* root = v.front();


		compute_lengths(root, 0);

		sort(codes_table_.begin(), codes_table_.end(), [](code& a, code& b) {
			return b.len_ > a.len_;
			});

		return;
	}


	void compute_lengths(const node* p, uint32_t len) {
		if (p->l_ == nullptr) {
			code c = { p->sym_, len, 0 };
			codes_table_.push_back(c);
		}
		else {
			compute_lengths(p->l_, len + 1);
			compute_lengths(p->r_, len + 1);
		}
	}

	void _compute_lengths(node* r, uint32_t len) {
		if (r->l_ == NULL) {
			T s = r->sym_;
			code c = { s, len, 0 };

			codes_table_.push_back(c);
		}
		else {
			compute_lengths(r->l_, len + 1);
			compute_lengths(r->r_, len + 1);
		}
	}

	void compute_canonical_codes() {
		uint32_t curr_len=0, curr_val=0;

		for (auto& e : codes_table_) {
			e.val_ = curr_val <<= (e.len_ - curr_len);

			curr_len = e.len_;
			curr_val++;

		}

	}

	void print_canonical_codes() {

		for (auto& e : codes_table_) {
			std::cout << e.sym_ << " , " << e.len_ << " , " << std::bitset<16>(e.val_).to_string().substr(16 - e.len_) << std::endl;
		}

	}


};