#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<omp.h>
#include<time.h>
#include "parse_input.c"
#include <mpi.h>

int min(int a, int b) {
  if (a<b) {
    return a;
  } else {
    return b;
  }
}


int main(int argc,char* argv[]) {
  for (int i=0; i<5; i++) {
  //MPI_Init(&argc, &argv);
  char filename[100] = "examples/32.in";
  FILE *fp = fopen(filename, "rb");
  int size;

  /*read in size of matrix*/
  fread(&size, sizeof(int), 1, fp);
  printf("size: %d\n",size);
  //parse adjacency matrix -> zeroes are converted to 'infinity'
  MatrixContainer matrix_container = get_Matrix(filename,size);
  printf("Time taken to parse: %f\n",matrix_container.time);
  int* matrix = matrix_container.matrix;
  int i,j,k;


  double t = MPI_Wtime();
  //size is array dimension
  //run algorithm
  for (k=0;k<size;k++) {
    for (i=0;i<size;i++) {
      for (j=0;j<size;j++) {
        matrix[i*size+j] = min(matrix[i*size+j], matrix[i*size+k] + matrix[k*size+j]);
      }
    }
  }
    double time_taken = MPI_Wtime() - t;
    printf("Time taken to run algorithm: %f\n",time_taken);
    /*
    //prints adjacency matrix
    for (i = 0; i <  size; i++) {
      printf("row%d: ",i);
      for (j = 0; j < size; j++) {
         printf("%d ", A[i][j]);
      }
      printf("\n");
    }
    */

  /*
  //prints pairwise shortest path result
  printf("\nresult\n");
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    //for (j = 0; j < size; j++) {
       printf("%d ", A[i][i%5]);
    //}
    printf("\n");
    */

  }
  return 0;
}
