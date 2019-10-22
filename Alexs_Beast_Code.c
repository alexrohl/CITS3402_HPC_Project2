#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"
#include "helper_functions.c"
#include <mpi.h>
#include <assert.h>


int main(int argc,char* argv[]) {
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
    int i,j,k;
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

  LocalData local_data;
  local_data.local_array = sub_array;
  //send 0th row and 0th column to arrays
  MPI_Bcast(krow, size, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(kcol, size, MPI_INT, 0, MPI_COMM_WORLD);



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
    local_data = update_local_array_with_k(local_data, global_index, num_local_elements, k, krow, kcol, size);
    snprintf(buf, 20, "AFTER_sub_array_%d", pid); // puts string into buffer
    //print_int_array(sub_array,num_local_elements,buf);

    MPI_Barrier(MPI_COMM_WORLD);
    //int* result = malloc(sizeof(int) * np * num_local_elements);

    //--------------MERGE K ROWS--------------
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
        for (int i=1; i<np; i++) {
           row_displs[i] = row_displs[i-1] + krow_entries[i-1];
        }
        col_displs[0] = 0;
        for (int i=1; i<np; i++) {
           col_displs[i] = col_displs[i-1] + kcol_entries[i-1];
        }

        //printf("Building %d row and column\n",k+1);
        //print_int_array(krow_entries,np,"krow_entries");
        //print_int_array(row_displs,np,"row_displs");
        //print_int_array(kcol_entries,np,"kcol_entries");
        //print_int_array(col_displs,np,"col_displs");





      }
      //use lengths to make k+1 row
      MPI_Gatherv(local_data.next_krow, local_data.next_krow_size, MPI_INT, krow, krow_entries, row_displs, MPI_INT, 0, MPI_COMM_WORLD);

      //use lengths to make k+1 col
      MPI_Gatherv(local_data.next_kcol, local_data.next_kcol_size, MPI_INT, kcol, kcol_entries, col_displs, MPI_INT, 0, MPI_COMM_WORLD);

      /* INCLUDE ELEMENTS FROM LEFTOVERS??
      if (pid==0) {
        matrix = merge_scattered_arrays(matrix, leftovers, lo, result, size);
      }
      */


      //--------------SHARE RESULT TO ALL PROCESSES-----------
      MPI_Bcast(krow, size, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(kcol, size, MPI_INT, 0, MPI_COMM_WORLD);
    }

  }

  //--------------MERGE ITERATION RESULT--------------
  int* result = malloc(sizeof(int) * np * num_local_elements);
  MPI_Gather(sub_array, num_local_elements, MPI_INT, result, num_local_elements, MPI_INT, 0, MPI_COMM_WORLD);
  if (pid==0) {
    matrix = merge_scattered_arrays(matrix, leftovers, lo, result, size);
    printf("merged\n");
  }

  //--------------PRINT RESULT--------------
  if (pid==0) {
    //LOGGING
    //Create File Name
    printf("logging...");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char output[60];
    sprintf(output,"outputs/%dvertices_%d%d%d_%d%d_%dprocesses_NEW.out",size,tm.tm_mday,tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour,tm.tm_min,np);
    FILE* fp = fopen(output, "w");
    print_matrix_to_file(matrix, size, fp);
    fclose(fp);
    printf("done\n");

    double runtime = MPI_Wtime() - t_start;
    printf("Time taken is %f seconds\n", runtime);
    //free(result);
    free(matrix);
  }
  free(sub_array);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}
