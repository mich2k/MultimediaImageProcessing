#include "main.h"

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}


int error(){
    printf("\nError\n");
    return EXIT_FAILURE;
}



int* file_to_arr (const char* filename, size_t* n){

    FILE* f = fopen(filename, "r");
    if(!f){
        error();
    }

    int* in_arr = NULL;

    for(size_t i = 0; ; i++){
        if(feof(f) || ferror(f)){
            break;
        }
        in_arr = realloc(in_arr,sizeof(int) * (i+1));

        int ret = fscanf(f, "%d", &in_arr[i]);
        
        (*n)++;
    }



    return in_arr;
}


void write_to_file(const char* out_filename, const int* sorted_arr, const size_t* n){

    FILE* f = fopen(out_filename, "w");

    if(!f || ferror(f)){
        error();
    }


    for(size_t i = 0; i < *n; i++){

        int r = fprintf(f, "%d\n", sorted_arr[i]);

    }


    return;
}




int main(int argc, char *argv[]){

    if ( argc != 3 ){
        error();
    }
    size_t n = 0;



    int* sorted_arr = file_to_arr(argv[1], &n);

    int* n_arr = malloc(n*sizeof(int));


    printf("Found: %lu elements\n\n", n);


    for(int i = 0; i < n; i++){
        printf("\n%d", sorted_arr[i]);
    }

    qsort(sorted_arr, n, sizeof(int), cmpfunc);

    printf("\n\nSORTED: ");

    for(int i = 0; i < n; i++){
        printf("\n%d", sorted_arr[i]);
    }

    write_to_file(argv[2], sorted_arr, &n);


    free(sorted_arr);
    free(n_arr);

    return EXIT_SUCCESS;

}