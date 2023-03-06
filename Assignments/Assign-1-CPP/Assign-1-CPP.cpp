// Assign-1-CPP.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <utility>

int cmpfunc(const void* a, const void* b)   //cmpfunc overflow-safe
{
    int32_t x = *(int32_t*)a;
    int32_t y = *(int32_t*)b;
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


// costruttore di coppia + operatore di assegnamento + distruttore
struct vector { // python-like module underscore for private attributes
    int32_t* nums_;
    int n_;
    int capacity_;


    // :: cpp operator for namespaces


    // constructor (same name as obj, no keyword needed)
    vector() {    // un identificatore all'interno di un metodo se non c'è tra i param etc  non serve specificare this
        this->nums_ = NULL;  // siccome si riferisce automaticamente all'oggetto
        this->n_ = 0;
        this->capacity_ = 0;
    }


    vector(int initial_size) {
        n_ = initial_size;
        capacity_ = initial_size;
        nums_ = (int32_t*) calloc(initial_size, sizeof(int32_t));

    }


    // In C++ you should almost never pass an object as a value and not as a reference
        // we may fall into a recursive constructor
        // we should always explicitly write down that we want actually pass the obj by value
    // So, everytime we use CONST OBJ_TYPE REFERENCE
        // SAME goes for String obj -> "const string&"
    vector(const vector& other) {
        n_ = other.n_;
        capacity_ = other.capacity_;
        nums_ = (int32_t*) malloc(sizeof(int32_t) * capacity_);
        for (size_t i = 0; i < n_; i++) {
            nums_[i] = other.nums_[i];
        }

    }






    // automatically gets called when the obj is out of scope
    ~vector() {
        free(this->nums_);
    }
    
    int size() const {
        return this->n_;
    }


    int32_t at(int i) const {
        assert(i >= 0 && i < n_);
        return nums_[i];
    }

    void sort() {
        qsort(nums_, n_, sizeof(int32_t), cmpfunc);
    }


    void push_back(int32_t num) {
        if (n_ == capacity_) {
            capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
            nums_ = (int32_t*)realloc(nums_, capacity_ * sizeof(int32_t));
            if (!nums_) {
                exit(1);
            }
            if (nums_ == NULL) {
                printf("Error: failed to allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }
        nums_[n_] = num;
        n_++;
    }


    void print() {
        for (size_t i = 0; i < n_; i++) {
            std::cout << nums_[i] << std::endl;
        }
    }


    // operator overloading [] square brackets
        // with reference i'm actually returning a var that is a reference to nums_[i] 
            // the cell memory, that is a lvalue and not the actual value

    // we need to delete the const keyword since eventually we are changing the value

    int32_t& operator[](int i) {
        return nums_[i];
    }

    // these two methods are not the same, since the first hidden parameter (this)
        // is this and const this in the second method

    const int32_t& operator[](int i) const {
        return nums_[i];
    }


    void write_vector(FILE* f,const vector& v) { // notice pass by reference, const since we do not need to edit it
        for (int i = 0; i < v.size(); i++) {
            fprintf(f, "%" PRId32 "\n", v[i]); // ERROR:  no operator '[]' matches these operands, operand types are: const vector [int]
                // CONST MANDATORY
        }
    }

    vector& operator=(vector other) { // copy-and-swap idiom

        swap(*this, other);
        return *this;
    }


    friend void swap(vector& left, vector& right) {
        using std::swap;
        swap(left.n_, right.n_);
        swap(left.capacity_, right.capacity_);
        swap(left.nums_, right.nums_);

    }




    /*

    vector& assign(const vector& other) { // reference


        // self-assignment prevention
            // is needed since i may have an array of vectors
            // and may happen ie during sorting

        if (this != &other) {   // address



            // destructor on old vector
            free(this->nums_);


            this->n_ = other.n_;
            this->capacity_ = other.capacity_;

            //mem alloc
            this->nums_ = (int32_t*)malloc(this->capacity_ * sizeof(int32_t));

            // copying files

            for (size_t i = 0; i < this->capacity_; i++) {
                this->nums_[i] = other.nums_[i];
            }

            return *this;

            // has to receive a const reference to the obj im copying
            // has to return a reference to an object, so a return *this
            // must be avoided self assignment or in any way managed
        }
    }
    */


};



struct number {
    int val_;
    number() : val_(7) { // Initialization 
            // DEFAULT CONSTRUCTOR
        //val_ = 7;
    }
};





// con int* val devo passare un puntatore dal chiamante, 
    // mettendo int &val vado a mettere default per reference e non per copia
    // comunque sfrutta il concetto di puntatore


// le reference le si creano e fanno riferimento ad un unica cosa sono quindi un alias

void raddoppia(int& val) {
    val *= 2;
}



void roba_file( char** argv, vector* v) {
    FILE* fin = fopen(argv[1], "r");
    if (fin == NULL) {
        printf("Error opening input file.\n");
        return;
    }

    FILE* fout = fopen(argv[2], "w");
    if (fout == NULL) {
        printf("Error opening output file.\n");
        fclose(fin);
        return;
    }


    while (1) {
        int32_t num;
        if (fscanf(fin, "%" SCNd32, &num) == 1) {
            (*v).push_back(num);
        }
        else if (feof(fin)) {
            break;
        }
        else {
            printf("Warning: incorrect data in input file.\n");
            break;
        }
    }

    v->sort();

    //(*v).sort();

    for (int i = 0; i < (*v).size(); i++) {
        fprintf(fout, "%" PRId32 "\n", (*v).at(i));
    }





}


int main(int argc, char** argv)
{
    

    printf("\n%s %s", argv[1], argv[2]);

    vector v = 0; // parameter to constr. 1-parameter syntax

    roba_file(argv, &v);


    vector v_2(10); // "as a function" syntax - more parameters

    vector v_3{ 10 }; // modern syntax



    // il costruttore DEFAULT è quello senza parentesi, non si può scrivere
    // vector v()

    //vector y = v; // copia elemento per elemento sui campi, quindi fa una shallow copy di nums
                    // siccome hanno lo stesso puntatore a nums_

    vector y = v; // INIT OEPRATION, v is passed as first param into vector constructor


    // WHEN CPP OBJS ARE USED:
    // 
    // 
        // THIS SYNTAX: AUTOMATIC MALLOC + number CONSTRUCTOR correctly CALLED
    number* myNumber = new number;

    // WRONG SYNTAX FOR OBJECTS:
    //
    //  not even Initialized
    number* noConstructorCalledNumber = (number*)malloc(sizeof(number));


    int value = myNumber->val_; // RIGHT
    int wrongValue = noConstructorCalledNumber->val_; // WRONG




    // destructors

    delete myNumber;
    delete noConstructorCalledNumber;




    // what if i need malloc of more numbers?

    number* moreNumbers = new number[10]; // here i alloc 10 numbers !! BRACKETS -> MORE THAN ONE

    delete[] moreNumbers; // new[] -> requires -> delete[] // brackets for brackets, deleter for more (10) objects




    // by changing with int_32& with reference in brackets operator 
    // we are able to change the value, since we have the cell memory

    y[0] = 2;  // NO CONST MANDATORY





}