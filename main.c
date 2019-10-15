#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"
#include "helper_functions.c"

// Temporary array for slave process
int a2[10000000];

int main(int argc,char* argv[]) {
  char filename[100] = "examples/16.txt";
  FILE *fp = fopen(filename, "r");
  int size, index_received, n_elements_recieved;

  /*read in size of matrix*/
  fscanf(fp, "%d", &size);
  printf("size: %d\n",size);
  //parse adjacency matrix -> zeroes are converted to 'infinity'
  MatrixContainer matrix_container = get_Matrix(filename,size);
  int* matrix = matrix_container.matrix;
  int i,j,k;

  //prints adjacency matrix
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    for (j = 0; j < size; j++) {
       printf("%d ", matrix[i*size+j]);
    }
    printf("\n");
  }

  //---------------PARTITION MATRIX------------------

  // master process
  if (pid == 0) {
    int index, i;
    int* a2
    elements_per_process = size * size / np;
    // check if more than 1 processes are run
    if (np > 1) {
        // distributes the portion of array
        // to child processes...
        for (i = 1; i < np - 1; i++) {
            index = i * elements_per_process;

            MPI_SendAll(index, matrix, elements_per_process, i);

        }
        // last process takes the remaining elements
        index = i * elements_per_process;
        int elements_left = n - index;
        MPI_SendAll(index, matrix, elements_left, i);

        //has own array...

    }
    // slave processes
    else {
      MPI_Recv(&index_received,
               1, MPI_INT, 0, 0,
               MPI_COMM_WORLD,
               &status);

      // stores the received array segment
      // in local array a2
      MPI_Recv(&a2, n_elements_recieved,
               MPI_INT, 0, 0,
               MPI_COMM_WORLD,
               &status);
      }

  int k =0;
  int* k_row = get_k_row(matrix,size,k);
  int* k_col = get_k_col(matrix,size,k);

  //--------------------------Loop accross iterations--------------------------
  for (k=0;k<size;k++) {


    // master process
    if (pid == 0) {
      a2 = update_local_array(a2, index_received, n_elements_received, k, k_col, k_row, size);
      //builds k_row and k_column
      for(update=0;update<2*size;update) {
        //build k_row
      }
    }
    // slave processes
    else {
      a2 = update_local_array(a2, index_received, n_elements_received, k, k_col, k_row, size);
    }


  //prints pairwise shortest path result
  printf("\nresult\n");
  for (i = 0; i <  size; i++) {
    printf("row%d: ",i);
    for (j = 0; j < size; j++) {
       printf("%d ", matrix[i*size+j]);
    }
    printf("\n");
  }

  return 0;
}
