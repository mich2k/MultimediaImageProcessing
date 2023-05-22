#include <algorithm>
#include <vector>

template <typename T>
struct huffman {
	struct node {
		T sym_;
		size_t prob_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(const T& sym, size_t prob) :
			sym_(sym), prob_(prob) {}
		node(node* a, node* b) :
			prob_(a->prob_ + b->prob_),
			left_(a), right_(b) {}
	};
	struct pnode {
		node* p_;
		pnode(node* p) : p_(p) {}
		operator node* () { return p_; }
		bool operator<(const pnode& rhs) const {
			return p_->prob_ > rhs.p_->prob_;
		}
	};
	struct code {
		T sym_;
		uint32_t len_, val_;
		bool operator<(const code& rhs) const {
			return len_ < rhs.len_;
		}
	};

	template <typename mapT>
	void create_table(const mapT& map) {
		std::vector<std::unique_ptr<node>> storage;
		std::vector<pnode> vec;
		for (const auto& x : map) {
			node* n = new node(x.first, x.second);
			storage.emplace_back(n);
			vec.push_back(n);
		}
		std::sort(begin(vec), end(vec));

		while (vec.size() > 1) {
			// Take the two least probable nodes and remove them from the vec
			pnode n1 = vec.back();
			vec.pop_back();
			pnode n2 = vec.back();
			vec.pop_back();
			// Combine them in a new node
			pnode n = new node(n1, n2);
			storage.emplace_back(n);
			// Add the new node back into the vector in the correct position
			auto it = lower_bound(begin(vec), end(vec), n);
			vec.insert(it, n);
		}
		node* root = vec.back();

		compute_lengths(root, 0);
		sort(codes_table_.begin(), codes_table_.end());

		for (auto e : codes_table_)
			std::cout << e.len_ << " " << e.sym_ << std::endl;

	}

	void compute_lengths(const node* p, uint32_t len) {
		if (p->left_ == nullptr) {
			code c = { p->sym_, len, 0 };
			codes_table_.push_back(c);
		}
		else {
			compute_lengths(p->left_, len + 1);
			compute_lengths(p->right_, len + 1);
		}
	}

	void compute_canonical_codes() {
		code cur = { 0, 0, 0 };
		for (auto& x : codes_table_) {
			x.val_ = cur.val_ <<= x.len_ - cur.len_;
			cur.len_ = x.len_;
			++cur.val_;
		}
	}

	std::vector<code> codes_table_;
};
