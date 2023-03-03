// Assign-1-CPP.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

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



struct vector { // attributo classe con underscore
    int32_t* nums_;
    int n_;
    int capacity_;


    // :: è l operatore di C++ che definisce il namespace in cui si trova


    // il costruttore si chiama come l'oggetto, non torna nulla
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


    // il distruttore non va chiamato, quando esce dallo scope viene chiamato automaticamente
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


}


int main(int argc, char** argv)
{

    printf("\n%s %s", argv[1], argv[2]);

    vector v = 0; // passa 10 al costruttore, notazione UNICO PARAMETRO

    roba_file(argv, &v);

    v.print();

    vector v_2(10); // NOTAZIONE FUNZIONALE, PIU PARAMETRI

    vector v_3{ 10 }; // NOTAZIONE MODERNA



    // il costruttore DEFAULT è quello senza parentesi, non si può scrivere
    // vector v()

    //vector y = v; // copia elemento per elemento sui campi, quindi fa una shallow copy di nums
                    // siccome hanno lo stesso puntatore a nums_

    //v.sort();
    //v.print();




    std::cout << "Hello World!\n";






}

// Per eseguire il programma: CTRL+F5 oppure Debug > Avvia senza eseguire debug
// Per eseguire il debug del programma: F5 oppure Debug > Avvia debug

// Suggerimenti per iniziare: 
//   1. Usare la finestra Esplora soluzioni per aggiungere/gestire i file
//   2. Usare la finestra Team Explorer per connettersi al controllo del codice sorgente
//   3. Usare la finestra di output per visualizzare l'output di compilazione e altri messaggi
//   4. Usare la finestra Elenco errori per visualizzare gli errori
//   5. Passare a Progetto > Aggiungi nuovo elemento per creare nuovi file di codice oppure a Progetto > Aggiungi elemento esistente per aggiungere file di codice esistenti al progetto
//   6. Per aprire di nuovo questo progetto in futuro, passare a File > Apri > Progetto e selezionare il file con estensione sln
