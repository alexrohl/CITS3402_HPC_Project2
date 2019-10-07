#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"

#define INFINITY 100000000

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
  MatrixContainer matrix_container = get_Matrix(filename,size);
  int* A = matrix_container.matrix;

  int i,j,k;
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    for (j = 0; j < size; j++) {
       printf("%d ", A[i*size + j]);
    }
    printf("\n");
  }

  for (k=0;k<size;k++) {
    for (i=0;i<size;i++) {
      for (j=0;j<size;j++) {
        A[i*size + j] = min(A[i*size + j], A[i*size + k] + A[k*size + j]);
      }
    }
  }

  printf("\nresult\n");
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    for (j = 0; j < size; j++) {
       printf("%d ", A[i*size + j]);
    }
    printf("\n");
  }

  return 0;
}
