#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"
#include "helper_functions.c"
#include <mpi.h>
#include <assert.h>


int main(int argc,char* argv[]) {
  int pid,np,size,lo,num_local_elements; //lo: left overs
  int *matrix;

  // ---INITIALIZE MPI---
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  // master process to get input matrix
  if (pid == 0) {
    char filename[100] = "examples/4.txt";
    FILE *fp = fopen(filename, "r");

    /*read in size of matrix*/
    fscanf(fp, "%d", &size);
    printf("Matrix Size: %d\n",size);
    num_local_elements = size*size/np;
    lo = size*size - np*num_local_elements;
    printf("Number of processes: %d\n",np);
    printf("Number of local elements: %d\n",num_local_elements);


    //parse adjacency matrix -> zeroes are converted to 'infinity'
    MatrixContainer matrix_container = get_Matrix(filename,size);
    matrix = matrix_container.matrix;
    int i,j,k;

    //prints adjacency matrix
    for (i = 0; i <  size; i++) {
      printf("row%d: ",i);
      for (j = 0; j < size; j++) {
         printf("%d ", matrix[i*size+j]);
      }
      printf("\n");
    }
  }

  //---------------PARTITION MATRIX------------------
  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&num_local_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&lo, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int *sub_array = malloc(sizeof(int) * num_local_elements);
  MPI_Scatter(&matrix[lo], num_local_elements, MPI_INT, sub_array,
              num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
  int global_index = lo + np*pid;

  //------------ADJUST ROOT TO INCLUDE LEFTOVERS------
  if (pid == 0) {
    global_index = 0;
    num_local_elements += lo;
    sub_array = malloc(sizeof(int) * (num_local_elements));
    for (int j=0;j<num_local_elements;j++){
      sub_array[j] = matrix[j];
    }
  }
  printf("Process %d has %d elements starting at index %d\n",pid,num_local_elements,global_index);
  char buf[20];
  snprintf(buf, 20, "BEFORE_sub_array_%d", pid); // puts string into buffer
  print_int_array(sub_array,num_local_elements,buf);


  //---------------GET k Row and Column-------------
  int *k_row = malloc(sizeof(int) * size);
  int *k_col = malloc(sizeof(int) * size);
  if (pid == 0) {
    k_row = get_k_row(matrix,size,0);
    k_col = get_k_col(matrix,size,0);
    print_int_array(k_row,size,"k_row");
    print_int_array(k_col,size,"k_col");
  }


  MPI_Bcast(k_row, size, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(k_col, size, MPI_INT, 0, MPI_COMM_WORLD);

  //--------------RUN ALGORITHMN ON SUBARRAY-----------

  sub_array = update_local_array(sub_array, global_index, num_local_elements, 0, k_col, k_row, size);
  snprintf(buf, 20, "AFTER_sub_array_%d", pid); // puts string into buffer
  print_int_array(sub_array,num_local_elements,buf);




  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}



/*
  // master process
  if (pid == 0) {
    int index, i;
    int* a2
    elements_per_process = size * size / np;
    MPI_Bcast(elements_per_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // check if more than 1 processes are run
    if (np > 1) {
        // distributes the portion of array
        // to child processes...
        MPI_Scatter(matrix, elements_per_process,  MPI_INT,
                    &a2, elements_per_process,  MPI_INT,
                    0, MPI_COMM_WORLD)


        for (i = 1; i < np - 1; i++) {
            index = i * elements_per_process;

            MPI_SendAll(index, matrix, elements_per_process, i);

        }
        // last process takes the remaining elements
        index = i * elements_per_process;
        int elements_left = n - index;
        MPI_SendAll(index, matrix, elements_left, i);

    }
    // slave processes
    else {
      MPI_Recv(&index_received,
               1, MPI_INT, 0, 0,
               MPI_COMM_WORLD,
               &status);

      }

  int k =0;
  int* k_row = get_k_row(matrix,size,k);
  int* k_col = get_k_col(matrix,size,k);

  //--------------------------Loop accross iterations--------------------------
  //for (k=0;k<size;k++) {


    // master process
    if (pid == 0) {
      //BROADCAST k_row and k_col
      MPI_Bcast(k_row, size, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(k_col, size, MPI_INT, 0, MPI_COMM_WORLD);


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
*/
