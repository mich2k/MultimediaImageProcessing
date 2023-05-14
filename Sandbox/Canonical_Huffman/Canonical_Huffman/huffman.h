#include <vector>
#include <algorithm>
#include <map>
using namespace std;

/*

	STEP:
		1.	introdurre template
		2.	definire oggetti: huffman, nodo
		3.	

*/
template<typename T>
struct huffman {


	struct code {
		T sym_;
		uint32_t len_, val_;

		//code(T& sym, uint32_t len, uint32_t val) {
		//	sym_ = sym;
		//	len_ = len;
		//	val_ = val;
		//}

	};

	std::vector<code> codes_table_;


	struct node {
		T sym_;
		size_t prob_;
		node* l_ = nullptr;
		node* r_ = nullptr;

		// 1 costruttore.
		// nodo generato da simbolo e probabilità

		node(const T& sym, size_t prob) {
			sym_ = sym;
			prob_ = prob;
		}

		// 2 costruttore
		// nodo posto come root dati due puntatori a nodo

		node(node* a, node* b) {
			sym_ = 0;
			prob_ = a->prob_ + b->prob_;
			l_ = a;
			r_ = b;
		}
	};

	// non sappiamo il tipo di mappa
	template <typename Tmap>

	void create_table(Tmap& map) {
		vector<node*> v;
		for (auto& x : map) {
			node* n = new node(x.first, x.second); // c++ -> new = dyn alloc = pointer
			v.push_back(n);
		}

		sort(v.begin(), v.end(), [](node* a, node* b) { // lambda function
			return a->prob_ > b->prob_;
			});

		while (v.size() > 1) {
			node* n1 = v.back();
			v.pop_back();
			node* n2 = v.back();
			v.pop_back();

			node* n = new node(n1, n2); // new -> mandatory


			// invece di sortare ogni volta, inserisce in modo ordinato

			auto it = lower_bound(v.begin(), v.end(), n, [](node* a, node* b){
				return a->prob_ > b->prob_;
				});

			cout << "iterator pos: " << v.begin() - it << endl;
			v.insert(it, n);

		}

		// dovrebbe back() == front() essendoci un solo elem
		node* root = v.back();

		compute_lengths(root, 0);

		cout << "Tables no sort" << endl;
		for (auto& x : codes_table_) {
			cout << x.sym_ << " " << x.len_ << endl;
		}
		cout << endl << "Tables sort" << endl;

		sort(codes_table_.begin(), codes_table_.end(), [](auto& p1, auto& p2){
				return p2.len_ > p1.len_;
			});

		}

	void compute_lengths(node* p, uint32_t len) {
		if (p->l_ == nullptr) {
			T s = p->sym_;
			code x = { s, len, 0 }; // GRAFFE: inizializzazione con lista multipla per la struttura
			// Le graffe riassumono questa classica init di una struct C-like
			// nota: le graffe non centrano nulla coi costruttori
			//		code x;
			//		x.sym_ = s;
			//		x.len_ = len;
			//		x.val_ = 0;
			codes_table_.push_back(x);
		}
		else {
			compute_lengths(p->l_, len + 1);
			compute_lengths(p->r_, len + 1);
		}
	}



};