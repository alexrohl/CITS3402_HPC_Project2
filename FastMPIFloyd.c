//Written by Alex Rohl 22233158

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"
#include "helper_functions.c"
#include <mpi.h>
#include <assert.h>


int main(int argc,char* argv[]) {
  int i, j, k;
  int pid,np,size,lo,num_local_elements; //lo: left overs
  int *leftovers;
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

  //-----------BROADCAST SIZES---------------
  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&num_local_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&lo, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int *matrix = malloc(sizeof(int) * size*size);
  int *krow = malloc(sizeof(int) * size);
  int *kcol = malloc(sizeof(int) * size);

  //------get input matrix---------
  if (pid == 0) {
    //parse adjacency matrix (zeroes are converted to 'infinity')
    MatrixContainer matrix_container = get_Matrix(argv[1],size);
    matrix = matrix_container.matrix;
    //prints adjacency matrix
    //print_matrix(matrix,size);
    krow = get_k_row(krow, matrix, size, 0);
    kcol = get_k_col(kcol, matrix, size, 0);
  }
  double t_start = MPI_Wtime();


  //---------------PARTITION MATRIX------------------
  MPI_Bcast(matrix, size*size, MPI_INT, 0, MPI_COMM_WORLD);
  int *sub_array = malloc(sizeof(int) * num_local_elements);
  MPI_Scatter(&matrix[lo], num_local_elements, MPI_INT, sub_array,
              num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
  int global_index = lo + num_local_elements*pid;
  printf("Process %d has %d elements starting at index %d\n",pid,num_local_elements,global_index);

  //send 0th row and 0th column to partitions
  MPI_Bcast(krow, size, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(kcol, size, MPI_INT, 0, MPI_COMM_WORLD);

  //-------------SEND LEFT OVERS TO ROOT-------------
  char buf[20];
  if(pid==0) {
    if (lo>0) {
      sub_array = malloc(sizeof(int)*(lo+num_local_elements));
      for (i=0;i<lo+num_local_elements;i++) {
        sub_array[i] = matrix[i];
      }
      global_index = 0;
      num_local_elements+=lo;
    }
  }

  LocalData local_data;
  local_data.local_array = sub_array;

  //--------------LOOPING OVER ITERATIONS--------------
  for(k = 0;k<size;k++) {

    //--------------RUN ALGORITHMN ON SUBARRAYS-----------
    local_data = update_local_array_with_k(local_data, global_index, num_local_elements, k, krow, kcol, size);

    //--------------MERGE THE NEXT KROW AND K COL--------------
    if (k<size-1) {
      //Gather krow entries
      int* krow_entries = malloc(sizeof(int) * np);
      MPI_Gather(&local_data.next_krow_size,1,MPI_INT,krow_entries,1,MPI_INT, 0, MPI_COMM_WORLD);

      //Gather kcol entries
      int* kcol_entries = malloc(sizeof(int) * np);
      MPI_Gather(&local_data.next_kcol_size,1,MPI_INT,kcol_entries,1,MPI_INT, 0, MPI_COMM_WORLD);

      //get displacements
      int* row_displs = malloc( np * sizeof(int) );
      int* col_displs = malloc( np * sizeof(int) );
      if (pid==0) {
        row_displs[0] = 0;
        for (i=1; i<np; i++) {
           row_displs[i] = row_displs[i-1] + krow_entries[i-1];
        }
        col_displs[0] = 0;
        for (i=1; i<np; i++) {
           col_displs[i] = col_displs[i-1] + kcol_entries[i-1];
        }
      }

      //use displacements to make k+1 row
      MPI_Gatherv(local_data.next_krow, local_data.next_krow_size, MPI_INT, krow, krow_entries, row_displs, MPI_INT, 0, MPI_COMM_WORLD);

      //use displacements to make k+1 col
      MPI_Gatherv(local_data.next_kcol, local_data.next_kcol_size, MPI_INT, kcol, kcol_entries, col_displs, MPI_INT, 0, MPI_COMM_WORLD);

      //--------------SHARE RESULT TO ALL PROCESSES-----------
      MPI_Bcast(krow, size, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(kcol, size, MPI_INT, 0, MPI_COMM_WORLD);
    }
  }

  //--------------MERGE RESULT AFTER K ITERATIONS--------------
  int* indices = malloc(sizeof(int) * np);
  int* num_local_elements_list = malloc(sizeof(int) * np);
  MPI_Gather(&global_index,1,MPI_INT,indices,1,MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Gather(&num_local_elements,1,MPI_INT,num_local_elements_list,1,MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Gatherv(local_data.local_array, num_local_elements, MPI_INT, matrix, num_local_elements_list, indices, MPI_INT, 0, MPI_COMM_WORLD);

  //--------------PRINT RESULT--------------
  if (pid==0) {
    //LOGGING
    //Create File Name
    printf("logging...");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char output[60];
    sprintf(output,"outputs/%dvertices_%d%d%d_%d%d_%dprocesses_FAST.out",size,tm.tm_mday,tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour,tm.tm_min,np);
    FILE* fp = fopen(output, "w");
    print_matrix_to_file(matrix, size, fp);
    fclose(fp);
    printf("done\n");
    double runtime = MPI_Wtime() - t_start;
    printf("Time taken is %f seconds\n", runtime);
    free(matrix);

  }
  MPI_Barrier(MPI_COMM_WORLD);
  free(sub_array);
  MPI_Finalize();
}
