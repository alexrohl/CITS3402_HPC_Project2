#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//assume 1D matrix...
int* get_k_row(int* matrix, int size, int k) {
  //printf("%d row ",k);
  int *row = malloc(size*sizeof(int));
  int i,j;
  j=0;
  for (i=k*size ; i < (k+1)*size ; i++) {
    row[j] = matrix[i];
    j++;
    //printf("%d ",row[j]);
  }
  //printf("\n");
  return row;
}

//assume 1D matrix...
int* get_k_col(int* matrix, int size, int k) {
  //printf("%d col ",k);
  int *col = malloc(size*sizeof(int));
  int i,j;
  j=0;
  for (i=k; i < size*size ; i+=size) {
    col[j] = matrix[i];
    j++;
    //printf("%d ",col[j]);
  }
  //printf("\n");
  return col;
}

int min(int a, int b) {
  if (a<b) {
    return a;
  } else {
    return b;
  }
}


void MPI_SendAll(int index, int* matrix, int elements_per_process, int i) {
  MPI_Send(&index,
           1, MPI_INT, i, 0,
           MPI_COMM_WORLD);

  MPI_Send(&matrix[index],
           elements_per_process,
           MPI_INT, i, 0,
           MPI_COMM_WORLD);

  MPI_Send(&elements_per_process,
           1, MPI_INT, i, 0,
           MPI_COMM_WORLD);

}


//Now we've received the elements, we want to update the array locally and send relevant information for the next processes
int* update_local_array(int* a2, int index_received, int n_elements, int k, int* k_col, int* k_row, int size) {
  int global_index,i,j;
  int ind = 0;
  //build: [row/column index value]
  int[3] build;


  //loop through local array
  for (global_index=index_received;global_index<n_elements;global_index++) {
    i = global_index/size;
    j = global_indexd%size;
    a2[ind] = min(a2[ind], k_col[i] + k_row[j]);

    //checks if we have an element that needs to be sent to master to build k_row/k_col
    if (i==k+1) {
      build = {0,j,a2[ind]};
      MPI_Send(build,
               3,
               MPI_INT, 0, 0,
               MPI_COMM_WORLD);
    }

    //checks if we have an element that needs to be sent to master to build k_row/k_col
    if (i==k+1) {
      build = {0,i,a2[ind]};
      MPI_Send(build,
               3,
               MPI_INT, 0, 0,
               MPI_COMM_WORLD);
    }

  }
  return a2;
}

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
