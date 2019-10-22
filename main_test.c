#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>
#include "parse_input.c"
#include "helper_functions.c"

void print_matrix(int *matrix, int size);
void floyd(int *sub_array, int size, int pid, int np);
int owner(int k, int np, int size);
void copy_row(int *sub_array, int size, int np, int *row_k, int k);


int main(int argc, char* argv[]) {
  if(argc == 1) {
    fprintf(stderr, "No Extra Command Line Argument Passed Other Than Program Name\n");
    fprintf(stderr, "Usage: \n");
    exit(1);
  }

  int i, j, k;
  int pid, np, size;
  int num_elements, num_local_elements, lo;  //lo: left overs
  int *matrix, *sub_array, *leftovers;
  FILE *fp;

  // ---INITIALIZE MPI---
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  // ----get relevant sizes----
  if (pid == 0) {
    FILE *fp = fopen(argv[1], "rb"); // rb for opening non-text files

    /*read in size of matrix*/
    fread(&size, sizeof(int), 1, fp);
    printf("Matrix Size: %d\n", size);
    num_elements = size * size;
    num_local_elements = num_elements / np;
    lo = num_elements - (np * num_local_elements);
    printf("Number of processes: %d\n", np);
    printf("Number of elements per processor: %d\n", num_local_elements);
  }

  //-----------BROADCAST SIZES---------------
  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&num_local_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&lo, 1, MPI_INT, 0, MPI_COMM_WORLD);

  matrix = malloc((size*size) * sizeof(int));

  //------get input matrix---------
  if (pid == 0) {
    //parse adjacency matrix (zeroes are converted to 'infinity')
    MatrixContainer matrix_container = get_Matrix(argv[1], size);
    matrix = matrix_container.matrix;
    print_matrix(matrix, size);
  }

  double t_start = MPI_Wtime();
  //---------------PARTITION MATRIX------------------
  sub_array = malloc(num_local_elements * sizeof(int)) ;
  MPI_Scatter(&matrix[lo], num_local_elements, MPI_INT, sub_array, num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
  int global_index = lo + num_local_elements*pid;
  printf("Process %d has %d elements starting at index %d\n", pid, num_local_elements, global_index);

  //-------------SEND LEFT OVERS TO ROOT-------------
  char buf[20];
  if(pid == 0) {
    if (lo > 0) {
      leftovers = malloc(lo * sizeof(int));
      for (i = 0; i < lo; i++) {
        leftovers[i] = matrix[i];
      }
    }
  }

  //--------------LOOPING OVER ITERATIONS--------------
  for(k = 0; k<size; k++) {
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

  //   //--------------MERGE ITERATION RESULT--------------
  //   MPI_Gather(sub_array, num_local_elements, MPI_INT, result, num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
  //   if (pid==0) {
  //     matrix = merge_scattered_arrays(matrix, leftovers, lo, result, size);
  //   }
  //
  //   //--------------SHARE RESULT TO ALL PROCESSES-----------
  //   MPI_Bcast(matrix, size*size, MPI_INT, 0, MPI_COMM_WORLD);
  //
  // }
  //
  // //--------------PRINT RESULT--------------
  // if (pid==0) {
  //   //LOGGING
  //   //Create File Name
  //   printf("logging...");
  //   time_t t = time(NULL);
  //   struct tm tm = *localtime(&t);
  //   char output[50];
  //   sprintf(output,"outputs/%dvertices_%d%d%d_%d%d_%dprocesses.out",size,tm.tm_mday,tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour,tm.tm_min,np);
  //   FILE* fp = fopen(output, "w");
  //   print_matrix(matrix, size, fp);
  //   fclose(fp);
  //   printf("done\n");
  //
  //   double runtime = MPI_Wtime() - t_start;
  //   printf("Time taken is %f seconds\n", runtime);
  //   //free(result);
  //   free(matrix);
  // }
  // free(sub_array);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
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

void floyd(int *sub_array, int size, int pid, int np) {
  int global_k, local_i, global_j, temp;
  int root;
  int* row_k = malloc(size * sizeof(int));

  for (global_k = 0; global_k < size; global_k++) {
    root = owner(global_k, np, size);
    if (pid == root) {
      copy_row(sub_array, size, np, row_k, global_k);
    }
    MPI_Bcast(row_k, size, MPI_INT, root, MPI_COMM_WORLD);
    for (local_i = 0; local_i < size/np; local_i++)
      for (global_j = 0; global_j < size; global_j++) {
        temp = sub_array[local_i * size + global_k] + row_k[global_j];
        if (temp < sub_array[local_i * size + global_j])
           sub_array[local_i * size + global_j] = temp;
      }
  }
}

// Reurn the rank of the process that owns global row k
int owner(int k, int np, int size) {
  return k/(size/np);
}

// Copy the row with the global subscript k into row_k
void copy_row(int *sub_array, int size, int np, int *row_k, int k) {
  int j;
  int local_k = k % (size/np);

  for (j = 0; j < size; j++) {
    row_k[j] = sub_array[local_k * size + j];
  }
}


//MPI_Scatter(global_matrix, num_elements_per_proc, MPI_INT, local_array, num_elements_per_proc, MPI_INT, 0, comm);
//MPI_Gather(local_array, num_elements_per_proc, MPI_INT, global_matrix, num_elements_per_proc, MPI_INT, 0, comm);