#include <vector>
#include <algorithm>
#include <map>
#include <bitset>
#define BITS 8


using namespace std;

/*

	STEP:
		1.	introdurre template
		2.	definire oggetti: huffman, nodo, code
		3.	metodo create_table
		4.	metodo compute_lengths

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
			// senza l'init. non saranno posti a null e non so quando 
			// mi trovo nelle foglie durante il calcolo delle lunghezze
		node* l_ = nullptr; // INIZIALIZZAZIONE OBBLIGATORIA
		node* r_ = nullptr; // INIZIALIZZAZIONE OBBLIGATORIA

		// 1 costruttore.
		// nodo generato da simbolo e probabilit�

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
			// NOTA: lower_bound vuole:
			//
			//	-	std::lower_bound
			//	-	4 PARAMETRI: beg, end, oggetto da inserire, comparatore

			auto it = lower_bound(v.begin(), v.end(), n, [](node* a, node* b){
				return a->prob_ > b->prob_;
				});

			v.insert(it, n); // it iterator returned from lower_bound insertion

		}

		// dovrebbe back() == front() essendoci un solo elem
		// l'ultimo rimasto sarà la radice dell'albero
		node* root = v.back();

		compute_lengths(root, 0);


		sort(codes_table_.begin(), codes_table_.end(), [](auto& p1, auto& p2){
				return p2.len_ > p1.len_;
			});

		for (auto& e : codes_table_)
			cout <<  e.sym_ << " " << e.len_ << endl;
		}


	// deve avere 2 parametri, nodep e len
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

	// code vector is all i need, public method
	void compute_canonical_codes() {
		uint32_t curr_len = 0, curr_val = 0;

		for (auto& e : codes_table_) {
			e.val_ = curr_val <<= (e.len_ - curr_len);	// shifto a sx la differenza di lunghezza
														// da cui mi trovo a quella precedente
														// naturalmente quella precedente sar� minore di quella attuale
			// e.val_ = 1 <<= ( 2 - 1 ) // mi trovo a 2, prima ero a uno, shifto di una posizione
			
			curr_len = e.len_;	

			curr_val++;

		}
	}

	void print_canonical_codes() {
		cout << "\nCanonical codes computed: " << endl;


		for (auto& e : codes_table_) {
			cout << "sym: " << e.sym_ << ", len: " << e.len_ << ", c_code: ";
			cout << bitset<BITS>(e.val_).to_string().substr(BITS - e.len_) << endl;
		}
	}


};