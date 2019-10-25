#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>

#define INFINITY 1000000
#define MAX_LINE_LEN 256

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) ((id+1)*(n)/(p) - 1)
#define BLOCK_SIZE(id,p,n) ((id+1)*(n)/(p) - (id)*(n)/(p))
#define BLOCK_OWNER(index,p,n) (((p)*((index)+1)-1)/(n))

void floyd(int **matrix, int n, int pid, int p);
int **parse_Matrix(char *filename);
void print_Matrix(int **matrix, int size);
void print_array(int *sub_array, int size, int np);
int min(int a, int b);



int main(int argc, char* argv[]) {
  if(argc == 1) {
    fprintf(stderr, "No Extra Command Line Argument Passed Other Than Program Name\n");
    fprintf(stderr, "Usage: \n");
    exit(1);
  }

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
    int **matrix = malloc(n * sizeof(int *));
    for (i = 0; i < n; i++) {
      matrix[i] = malloc(n * sizeof(int));
    }
    matrix = parse_Matrix(filename);
    print_Matrix(matrix, n);
    printf("Number of processes: %d\n", np);
    printf("%d\n", matrix[0][1]);
    floyd(matrix, n, pid, np);
  }


  //printf("%d\n", matrix[0][1]);


  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}

// Distributed version of Floyd Marshall algorithm
void floyd(int **matrix, int n, int pid, int p) {
  int i, j, k;
  int offset; /* Local index of broadcast row */
  int root; /* Process controlling row to be bcast */
  int *temp; /* Holds the broadcast row */
  temp = malloc(n * sizeof(int));

  for (k = 0; k < n; k++) {
    root = BLOCK_OWNER(k, p, n);
    if (root == pid) {
      offset = k - BLOCK_LOW(pid, p, n);
      for (j = 0; j < n; j++) {
        temp[j] = matrix[offset][j];
      }
    }
    MPI_Bcast(temp, n, MPI_INT, root, MPI_COMM_WORLD);
    for (i = 0; i < BLOCK_SIZE(pid, p, n); i++)
      for (j = 0; j < n; j++)
        matrix[i][j] = min(matrix[i][j], matrix[i][k]+temp[j]);
        printf("%d ", matrix[i][j]);
  }
  free(temp);
}

int **parse_Matrix(char *filename) {
  int i_elem, i, j, n, **matrix;
  char type[MAX_LINE_LEN];
  FILE *fp = fopen(filename, "rb");
  fread(&n, sizeof(int), 1, fp);

  matrix = malloc(n * sizeof(int *));
  for (i = 0; i < n; i++) {
    matrix[i] = malloc(n * sizeof(int));
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      fread(&i_elem, sizeof(int), 1, fp);
      if (i == j) i_elem = 0;
      else if (i_elem == 0) i_elem = INFINITY;
      matrix[i][j] = i_elem;
    }
  }
  return matrix;
}

void print_Matrix(int **matrix, int n) {
  int i, j;
  for (i = 0; i < n; i++) {
     for (j = 0; j < n; j++) {
       if (matrix[i][j] == INFINITY)
          printf("i ");
       else
          printf("%d ", matrix[i][j]);
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

int min(int a, int b) {
  if (a<b) {
    return a;
  } else {
    return b;
  }
}
