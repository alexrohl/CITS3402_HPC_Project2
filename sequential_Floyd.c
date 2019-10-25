#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<omp.h>
#include<time.h>
#include "parse_input.c"
#include"helper_functions.c"
#include <mpi.h>

int main(int argc,char* argv[]) {
  FILE *fp = fopen(argv[1], "rb");
  int size;

  /*read in size of matrix*/
  fread(&size, sizeof(int), 1, fp);
  printf("size: %d\n",size);
  //parse adjacency matrix -> zeroes are converted to 'infinity'
  MatrixContainer matrix_container = get_Matrix(argv[1],size);
  printf("Time taken to parse: %f\n",matrix_container.time);
  int* matrix = matrix_container.matrix;
  int i,j,k;


  double time_taken = MPI_Wtime();
  //size is array dimension
  //run algorithm
  for (k=0;k<size;k++) {
    for (i=0;i<size;i++) {
      for (j=0;j<size;j++) {
        matrix[i*size+j] = min(matrix[i*size+j], matrix[i*size+k] + matrix[k*size+j]);
      }
    }
  }


  //LOGGING
  //Create File Name
  printf("logging...");
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char output[60];
  sprintf(output,"outputs/%dvertices_%d%d%d_%d%d_1processes_SEQ.out",size,tm.tm_mday,tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour,tm.tm_min);
  fp = fopen(output, "w");
  print_matrix_to_file(matrix, size, fp);
  fclose(fp);
  printf("done\n");

  double runtime = MPI_Wtime() - time_taken;
  printf("Time taken is %f seconds\n", runtime);
  //free(result);
  free(matrix);

  
  return 0;
}
