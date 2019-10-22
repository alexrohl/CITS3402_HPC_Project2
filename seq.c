#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<omp.h>
#include<time.h>
#include <mpi.h>
#include "parse_input.c"

int min(int a, int b);
void print_matrix(int *matrix, int size);

int main(int argc,char* argv[]) {
  MPI_Init(NULL, NULL);
  int i,j,k;
  char filename[100] = "examples/32.in";
  FILE *fp = fopen(filename, "rb");
  int size;

  /*read in size of matrix*/
  fread(&size, sizeof(int), 1, fp);
  printf("SIZE: %d\n",size);
  //parse adjacency matrix -> zeroes are converted to 'infinity'
  MatrixContainer matrix_container = get_Matrix(filename,size);
  printf("Time taken to parse: %f\n",matrix_container.time);
  int* matrix = matrix_container.matrix;

  for (k=0;k<size;k++) {
    for (i=0;i<size;i++) {
      for (j=0;j<size;j++) {
        matrix[i*size+j] = min(matrix[i*size+j], matrix[i*size+k] + matrix[k*size+j]);
      }
    }
  }

  print_matrix(matrix, size);
  
  MPI_Finalize();
  return 0;
}

int min(int a, int b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

void print_matrix(int *matrix, int size) {
  int i, j;
  for (i = 0; i < size; i++) {
     for (j = 0; j < size; j++) {
       if (matrix[i*size+j] == INFINITY)
          printf("i ");
       else
          printf("%d ", matrix[i*size+j]);
     }
     printf("\n");
  }
}
