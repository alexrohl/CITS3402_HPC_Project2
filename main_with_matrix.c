#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"
#include "helper_functions.c"
#include <mpi.h>
#include <assert.h>


int main(int argc,char* argv[]) {
  int i, j, k;
  if(argc==1)
        printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
  if(argc>=2)
      {
    int pid,np,size,lo,num_local_elements; //lo: left overs
    int *leftovers;
    //char filename[100] = "examples/16.txt";
    FILE *fp;
    // ---INITIALIZE MPI---
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // ----get relevant sizes----
    if (pid == 0) {
      FILE *fp = fopen(argv[1], "rb");

      /*read in size of matrix*/
      fread(&size, sizeof(int), 1, fp);
      printf("Matrix Size: %d\n",size);
      num_local_elements = size*size/np;
      lo = size*size - np*num_local_elements;
      printf("Number of processes: %d\n",np);
      printf("Number of local elements: %d\n",num_local_elements);
    }

  //---------------PARTITION MATRIX------------------
  MPI_Bcast(matrix, size*size, MPI_INT, 0, MPI_COMM_WORLD);
  int *sub_array = malloc(sizeof(int) * num_local_elements);
  MPI_Scatter(&matrix[lo], num_local_elements, MPI_INT, sub_array,
              num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
  int global_index = lo + num_local_elements*pid;
  printf("Process %d has %d elements starting at index %d\n",pid,num_local_elements,global_index);

  //-------------SEND LEFT OVERS TO ROOT-------------
  char buf[20];
  if(pid==0) {
    if (lo>0) {
      leftovers = malloc(sizeof(int)*lo);
      for (i=0;i<lo;i++) {
        leftovers[i] = matrix[i];
      }
    }

  //--------------LOOPING OVER ITERATIONS--------------
  for(k = 0;k<size;k++) {

    //---------------PARTITION MATRIX------------------
    MPI_Bcast(matrix, size*size, MPI_INT, 0, MPI_COMM_WORLD);
    int *sub_array = malloc(sizeof(int) * num_local_elements);
    MPI_Scatter(&matrix[lo], num_local_elements, MPI_INT, sub_array,
                num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
    int global_index = lo + num_local_elements*pid;
    printf("Process %d has %d elements starting at index %d\n",pid,num_local_elements,global_index);

    //-------------SEND LEFT OVERS TO ROOT-------------
    char buf[20];
    if(pid==0) {
      if (lo>0) {
        leftovers = malloc(sizeof(int)*lo);
        for (int i=0;i<lo;i++) {
          leftovers[i] = matrix[i];
        }
      }
    }

    //--------------LOOPING OVER ITERATIONS--------------
    for(int k = 0;k<size;k++) {

      //--------------RUN ALGORITHMN ON LEFTOVERS-----------
      if(pid==0) {
        if (lo>0) {
          snprintf(buf, 20, "BEFORE_sub_array_%d", -1); // puts string into buffer
          //print_int_array(leftovers,lo,buf);
          leftovers = update_local_array_with_matrix(leftovers, 0, lo, k, matrix, size);
          snprintf(buf, 20, "AFTER_sub_array_%d", -1); // puts string into buffer
          //print_int_array(leftovers,lo,buf);
        }
      }

      //--------------RUN ALGORITHMN ON SUBARRAYS-----------
      snprintf(buf, 20, "BEFORE_sub_array_%d", pid); // puts string into buffer
      //print_int_array(sub_array,num_local_elements,buf);
      sub_array = update_local_array_with_matrix(sub_array, global_index, num_local_elements, k, matrix, size);
      snprintf(buf, 20, "AFTER_sub_array_%d", pid); // puts string into buffer
      //print_int_array(sub_array,num_local_elements,buf);

      MPI_Barrier(MPI_COMM_WORLD);
      int* result = malloc(sizeof(int) * np * num_local_elements);

      //--------------MERGE ITERATION RESULT--------------
      MPI_Gather(sub_array, num_local_elements, MPI_INT, result, num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
      if (pid==0) {
        matrix = merge_scattered_arrays(matrix, leftovers, lo, result, size);
      }

      //--------------SHARE RESULT TO ALL PROCESSES-----------
      MPI_Bcast(matrix, size*size, MPI_INT, 0, MPI_COMM_WORLD);

    }

    //--------------PRINT RESULT--------------
    if (pid==0) {
      print_matrix(matrix, size);
      free(matrix);
      //free(result);
    }
    free(sub_array);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
  }
}
