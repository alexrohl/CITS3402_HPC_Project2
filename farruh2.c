#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>
#include"helper_functions.c"

#define INFINITY 1000000
#define MAX_LINE_LEN 256

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) ((id+1)*(n)/(p))
#define BLOCK_SIZE(id,p,n) n*n/p
#define BLOCK_OWNER(index,p,n) index*p/n

void floyd(int *matrix, int n, int pid, int p);
int *parse_Matrix(int *matrix,char *filename);
void print_Matrix(int *matrix, int size);
void print_array(int *sub_array, int size, int np);
int min1(int a, int b);



int main(int argc, char* argv[]) {
  if(argc == 1) {
    fprintf(stderr, "No Extra Command Line Argument Passed Other Than Program Name\n");
    fprintf(stderr, "Usage: \n");
    exit(1);
  }
  double t_start = MPI_Wtime();
  int i, j, k;
  int pid, np, n, root = 0;
  char *filename = argv[1];

  // ---INITIALIZE MPI---
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  // Matrix is parsed in the root process (process 0)
  if (pid == root) {
    FILE *fp = fopen(filename, "rb");
    fread(&n, sizeof(int), 1, fp);
    printf("Matrix Size: %d\n", n);
    printf("Number of processes: %d\n", np);
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int *matrix = malloc(n*n * sizeof(int));

  if (pid == root) {
    matrix = parse_Matrix(matrix,filename);
    print_Matrix(matrix, n);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  //print_Matrix(matrix,n);

  MPI_Bcast(matrix, n*n, MPI_INT, 0, MPI_COMM_WORLD);
  //printf("bcast");
  //print_Matrix(matrix, n);
  if (pid==1) {
    print_Matrix(matrix, n);

  }
  printf("before floyd");
  //int *matrix2 = malloc(n*n * sizeof(int));
  floyd(matrix, n, pid, np);


  MPI_Barrier(MPI_COMM_WORLD);

  //--------------PRINT RESULT--------------
  if (pid==0) {
    printf("logging...");
    //print_Matrix(matrix, n);
    //LOGGING
    //Create File Name

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char output[70];
    sprintf(output,"outputs/%dvertices_%d%d%d_%d%d_%dprocs_FARRUH.out",n,tm.tm_mday,tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour,tm.tm_min,np);
    FILE* fp = fopen(output, "w");
    print_matrix_to_file(matrix, n, fp);
    fclose(fp);
    printf("done\n");

    double runtime = MPI_Wtime() - t_start;
    printf("Time taken is %f seconds\n", runtime);
    //free(result);
    free(matrix);
  }


  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}

// Distributed version of Floyd Marshall algorithm
void floyd(int *matrix, int n, int pid, int p) {
  int i, j, k;
  int offset; /* Local index of broadcast row */
  int root; /* Process controlling row to be bcast */
  int *temp; /* Holds the broadcast row */
  temp = malloc(n * sizeof(int));

  for (k = 0; k < n; k++) {
    root = BLOCK_OWNER(k, p, n);
    if (root == pid) {
      printf("block owner: %d\n",root);
      for (j = 0; j < n; j++) {
        temp[j] = matrix[k*n + j];
      }
      print_int_array(temp,n,"temp");
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(temp, n, MPI_INT, root, MPI_COMM_WORLD);
    for (i = BLOCK_LOW(pid,p,n); i < BLOCK_HIGH(pid,p,n); i++) {
      for (j = 0; j < n; j++) {
        //printf("b: %d ",matrix[i*n+j]);
        matrix[i*n+j] = min1(matrix[i*n+j], matrix[i*n+k]+temp[j]);
        //printf("a: %d ",matrix[i*n+j]);
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  free(temp);
  MPI_Barrier(MPI_COMM_WORLD);
  return;
}

int *parse_Matrix(int *matrix ,char *filename) {
  int i_elem, i, j, n;
  char type[MAX_LINE_LEN];
  FILE *fp = fopen(filename, "rb");
  fread(&n, sizeof(int), 1, fp);

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      fread(&i_elem, sizeof(int), 1, fp);
      if (i == j) i_elem = 0;
      else if (i_elem == 0) i_elem = INFINITY;
      matrix[i*n + j] = i_elem;
    }
  }
  return matrix;
}

void print_Matrix(int *matrix, int n) {
  int i, j;
  for (i = 0; i < n; i++) {
     for (j = 0; j < n; j++) {
       if (matrix[i*n + j] == INFINITY)
          printf("i ");
       else
          printf("%d ", matrix[i*n + j]);
     }
     printf("\n");
  }
}

void print_array(int *sub_array, int size, int np) {
  int i;
  for (i = 0; i < (size * size)/np; i++) {
    if (sub_array[i] == INFINITY)
      printf("i ");
    else
      printf("%d ", sub_array[i]);
  }
  printf("\n");
}

int min1(int a, int b) {
  if (a<b) {
    return a;
  } else {
    return b;
  }
}
