#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include <assert.h>

void print_int_array(int * arr, int len, char name[64]) {
    printf("Array %s: [",name);

    int i;
    for (i=0; i<len; i++) {
      printf("%d,", arr[i]);
    }
    printf("] has length %d\n",len);
    return;
}
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
int * update_local_array(int* local_array, int index_received, int n_elements, int k, int* k_col, int* k_row, int size) {
  //printf("local_function_call");
  assert(k==0);
  assert(index_received<=size*size);
  assert(n_elements<=size*size);

  int global_index,i,j;
  int ind = 0;
  //print_int_array(k_col,size,"k_col");
  //print_int_array(k_row,size,"k_row");
  //build: [row/column index value]


  //loop through local array
  for (global_index=index_received;global_index<index_received+n_elements;global_index++) {
    i = global_index/size;
    j = global_index%size;
    //printf("BEFORE i: %d, j: %d, val: %d\n",i,j,local_array[ind]);
    local_array[ind] = min(local_array[ind], k_col[i] + k_row[j]);
    //printf("AFTER i: %d, j: %d, val: %d\n",i,j,local_array[ind]);
    ind++;
    /*
    //checks if we have an element that needs to be sent to master to build k_row/k_col
    if (i==k+1) {
      int build[3] = {0,j,a2[ind]};
      MPI_Send(build,
               3,
               MPI_INT, 0, 0,
               MPI_COMM_WORLD);
    }

    //checks if we have an element that needs to be sent to master to build k_row/k_col
    if (i==k+1) {
      int build[3] = {0,0,0};//{0,i,a2[ind]};
      MPI_Send(build,
               3,
               MPI_INT, 0, 0,
               MPI_COMM_WORLD);
    }

  }
  */
  }
  return local_array;
}

int * append_int_to_array(int *arr1, int val1, int index){
  int* temp;
  /*append to values*/
  arr1[index]=val1;
  temp=realloc(arr1,(index+2)*sizeof(int));
  /*using temp*/
  if ( temp != NULL ) {
    arr1=temp;
  } else {
    free(arr1);
    printf("Error allocating memory!\n");
  }
  return arr1;
}
/*

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

*/
