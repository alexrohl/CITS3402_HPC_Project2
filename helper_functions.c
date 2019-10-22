#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include <assert.h>

int i, j, k;

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
int * update_local_array_with_matrix(int* local_array, int index_received, int n_elements, int k, int* matrix, int size) {
  //printf("local_function_call");
  assert(index_received<=size*size);
  assert(n_elements<=size*size);
  if (k==0) {
    for (i=0;i<n_elements;i++) {
      assert(matrix[index_received+i] == local_array[i]);
    }
  }

  int global_index,i,j;
  int ind = 0;
  //print_int_array(k_col,size,"k_col");
  //print_int_array(k_row,size,"k_row");
  //build: [row/column index value]


  //loop through local array
  for (global_index=index_received;global_index<index_received+n_elements;global_index++) {
    i = global_index/size;
    j = global_index%size;
    assert(i*size+k<size*size);
    assert(k*size+j<size*size);
    local_array[ind] = min(local_array[ind], matrix[i*size+k] + matrix[k*size+j]);
    // we compute A[i][j] = min(A[i][j], A[i][k] + A[k][j]) as follows###

    ind++;

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

void print_matrix_to_file(int * matrix, int size, FILE *fp) {//prints adjacency matrix
  fprintf(fp,"%d\n",size);
  for (i = 0; i <  size; i++) {
    //fprintf("row%d: ",i);
    for (j = 0; j < size; j++) {
       fprintf(fp,"%d ", matrix[i*size + j]);
    }
    //printf("\n");
  }
}

int * merge_scattered_arrays(int * matrix, int * leftovers, int lo, int* result, int size) {
  //assert(matrix[lo] == result[0]);
  if (lo > 0) {
    for (i=0;i<lo;i++) {
      matrix[i] = leftovers[i];
    }
  }
  for (i=lo; i<size*size; i++) {
    matrix[i] = result[i-lo];
  }
  assert(matrix[lo]==result[0]);
  return matrix;
}
