#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_input.c"
#include "helper_functions.c"
#include <mpi.h>


int main(int argc,char* argv[]) {
  int size = 50;
// ---INITIALIZE MPI---
  MPI_Init(&argc, &argv);
  // find out process ID,
  // and how many processes were started
  int pid,np;
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  int elements_per_proc,*rbuf;
  elements_per_proc = size/np;
  int left_overs = size - elements_per_proc*np;
  MPI_Status status;

  //int *array = NULL;
  int*array = malloc(sizeof(int) * size);
  if (pid == 0) {
    printf("number of process %d\n",np);
    int i;
    for (i=0;i<size;i++) {
      array[i]=i;
    }
    print_int_array(array, size, "full array");
    rbuf = (int *)malloc(size*sizeof(int));
  }

  //MPI_Bcast(&elements_per_proc, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int *sub_array = malloc(sizeof(int) * elements_per_proc);
  MPI_Scatter(&array[left_overs], elements_per_proc, MPI_INT, sub_array,
              elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
  int i;
  //ROOT TO HANDLE LEFT OVERS
  if (pid == 0) {
    print_int_array(sub_array,elements_per_proc,"sub array 0");
    elements_per_proc += left_overs;
    //printf("%d, %d",left_overs,elements_per_proc);
    sub_array = malloc(sizeof(int) * (elements_per_proc+1));
    for (int j=0;j<elements_per_proc;j++){
      //printf("%d",j);
      sub_array[j] = j;
    }
  }
  //LEFT OVERS HANDLED
  printf("Hello from process %d\n",pid);
  //print_int_array(sub_array,elements_per_proc,"sub array");
  printf("\n");

  MPI_Gather(sub_array, elements_per_proc, MPI_INT, result, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

  if (pid == 0) {
    print_int_array(rbuf, size, "full array remerged");
    //free(rbuf);
    //free(array);
  }
  free(array);
  free(sub_array);
  // cleans up all MPI state before exit of process
  //MPI_Finalize();
}
