#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include <utility>

int cmpfunc(const void* a, const void* b)
{
    double x = *(double*)a;
    double y = *(double*)b;
    if (x < y) {
        return -1;
    }
    else if (x > y) {
        return 1;
    }
    else {
        return 0;
    }
}

template<typename T>
struct vector {
    T* nums_;
    int n_;
    int capacity_; // capacity of nums array

    vector() {
        printf("default constructor\n");
        nums_ = NULL;
        n_ = 0;
        capacity_ = 0;
    }
    vector(int initial_size) {
        printf("constructor (int)\n");
        nums_ = new T[initial_size];
        n_ = initial_size;
        capacity_ = initial_size;
    }
    vector(const vector& other) {
        printf("copy constructor\n");
        n_ = other.n_;
        capacity_ = other.capacity_;
        nums_ = new T[capacity_];
        for (size_t i = 0; i < n_; ++i) {
            nums_[i] = other.nums_[i];
        }
    }
    vector(vector&& other) {
        printf("move constructor\n");
        n_ = other.n_;
        capacity_ = other.capacity_;
        nums_ = other.nums_;
        other.nums_ = nullptr;
    }
    /*    vector &operator=(const vector &other) {
            printf("copy assignment\n");
            if (this != &other) {
                delete[] nums_;
                n_ = other.n_;
                capacity_ = other.capacity_;
                nums_ = new T[capacity_];
                for (size_t i = 0; i < n_; ++i) {
                    nums_[i] = other.nums_[i];
                }
            }
            return *this;
        }
        // other � una r-value reference
        vector &operator=(vector &&other) {
            printf("move assignment\n");
            if (this != &other) {
                delete[] nums_;
                n_ = other.n_;
                capacity_ = other.capacity_;
                nums_ = other.nums_;
                other.nums_ = nullptr;
            }
            return *this;
        }
        */
    vector& operator=(vector other) {
        swap(*this, other);
        return *this;
    }
    friend void swap(vector& left, vector& right) {
        using std::swap;
        swap(left.n_, right.n_);
        swap(left.capacity_, right.capacity_);
        swap(left.nums_, right.nums_);
    }
    ~vector() {
        printf("destructor\n");
        delete[] nums_;
    }
    void push_back(const T& num) {
        if (n_ == capacity_) {
            capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
            auto* tmp = new T[capacity_];
            if (tmp == NULL) {
                printf("Error: failed to allocate memory.\n");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < n_; ++i) {
                tmp[i] = nums_[i];
            }
            delete[] nums_;
            nums_ = tmp;
        }
        nums_[n_] = num;
        n_++;
    }
    int size() const {
        return n_;
    }
    T& at(int i) const {
        assert(i >= 0 && i < n_);
        return nums_[i];
    }

    T& operator[](int i) {
        return nums_[i];
    }
    const T& operator[](int i) const {
        return nums_[i];
    }
};

void scrivi_vettore(FILE* f, const vector<double>& v) {
    for (int i = 0; i < v.size(); i++) {
        fprintf(f, "%f\n", v[i]);
    }
}

struct esempio {
    int val_;
    esempio(int val = 0) : val_(val) {
        printf("esempio(%d)\n", val);
    }
};

auto crea_elementi()
{
    vector<esempio> out;
    out.push_back(6);
    out.push_back(-9);
    out.push_back(23);
    return out;
}

int main(int argc, char* argv[])
{
    vector<esempio> mio;

    mio = crea_elementi();

    return 0;





    /*
        if (argc != 3) {
            printf("Usage: sort_int <filein.txt> <fileout.txt>\n");
            return 1;
        }

        FILE *fin = fopen(argv[1], "r");
        if (fin == NULL) {
            printf("Error opening input file.\n");
            return 1;
        }

        FILE *fout = fopen(argv[2], "w");
        if (fout == NULL) {
            printf("Error opening output file.\n");
            fclose(fin);
            return 1;
        }

        vector<double> v;
        vector<int> x;

        while (1) {
            double num;
            if (fscanf(fin, "%lf", &num) == 1) {
                v.push_back(num);
            }
            else if (feof(fin)) {
                break;
            }
            else {
                printf("Warning: incorrect data in input file.\n");
                break;
            }
        }

        qsort(&v[0], v.size(), sizeof(double), cmpfunc);

        scrivi_vettore(fout, v);

        fclose(fin);
        fclose(fout);
    */
    return 0;
}