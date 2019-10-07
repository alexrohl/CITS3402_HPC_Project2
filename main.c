#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"

#define INFINITY 10000000000

int min(int a, int b) {
  if (a<b) {
    return a;
  } else {
    return b;
  }
}


int main(int argc,char* argv[]) {
  char filename[100] = "examples/16.txt";
  FILE *fp = fopen(filename, "r");
  int size;

  /*read in size of matrix*/
  fscanf(fp, "%d", &size);
  printf("size: %d\n",size);
  //parse adjacency matrix -> zeroes are converted to 'infinity'
  MatrixContainer matrix_container = get_Matrix(filename,size);
  int* matrix = matrix_container.matrix;
  int A[size][size];
  int i,j,k;

  //merge matrix into 2D array for simpler notation
  for (i = 0; i <  size; i++) {
    for (j = 0; j < size; j++) {
      A[i][j] = matrix[i*size+j];
    }
  }

  //prints adjacency matrix
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    for (j = 0; j < size; j++) {
       printf("%d ", A[i][j]);
    }
    printf("\n");
  }

  //run algorithm
  for (k=0;k<size;k++) {
    for (i=0;i<size;i++) {
      for (j=0;j<size;j++) {
        A[i][j] = min(A[i][j], A[i][k] + A[k][j]);
      }
    }
  }

  //prints pairwise shortest path result
  printf("\nresult\n");
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    for (j = 0; j < size; j++) {
       printf("%d ", A[i][j]);
    }
    printf("\n");
  }

  return 0;
}
