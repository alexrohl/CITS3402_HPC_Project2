/*
 * Floyd_Warshall_mpi.c
 *
 *  Created on: 4 jan 2012
 *      Author: fotis
 */

/*
    floyd_warshall()

    after calling this function dist[i][j] will the the minimum distance
    between i and j if it exists (i.e. if there's a path between i and j)
    or 0, otherwise
*/

#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#define MASTER 0
#define WORKTAG 1
#define DIETAG 2
#define n 300 /* Then number of nodes */

int dist[n][n]; /* dist[i][j] is the length of the edge between i and j if
            it exists, or 0 if it does not */

void printDist() {
    int i, j;
    printf("    ");
    for (i = 0; i < n; ++i)
        printf("%4c", 'A' + i);
    printf("\n");
    for (i = 0; i < n; ++i) {
        printf("%4c", 'A' + i);
        for (j = 0; j < n; ++j)
            printf("%4d", dist[i][j]);
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {

    int my_rank,                /* rank of process */
            num_procs,              /* number of processes */
            slice,
            ypoloipo=0;
    MPI_Status status;
    /* Start up MPI */
    MPI_Init(&argc,&argv);
    /* Find out number of process */
    MPI_Comm_size(MPI_COMM_WORLD,&num_procs);
    /* Find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

    ypoloipo= n%(num_procs-1);
    slice= (n-ypoloipo)/(num_procs-1);

    /* master code */
    if (my_rank == MASTER) {
      double t1,t2;
      int disable=0,t=3;
      int result[t];
      //initialize dist[][]
      int i, j;
      for (i = 0; i < n; ++i)
          for (j = 0; j < n; ++j)
              if (i==j)
                dist[i][j]=0;
              else
                dist[i][j]= (int)( 11.0 * rand() / ( RAND_MAX + 1.0 ) );// number creator 0-10

      printDist();// printout the new array
       t1 = MPI_Wtime();
          for(i=1;i<num_procs;i++)
         MPI_Send(&dist,n*n,MPI_INT,i,WORKTAG,MPI_COMM_WORLD);//send the array dist in every machine

      do {
         MPI_Recv(&result,t,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
         if (status.MPI_TAG == DIETAG)
            disable++;
         else
            if (dist[result[1]][result[2]]>result[0])
                dist[result[1]][result[2]]=result[0];
      } while (disable < num_procs-1);
       t2 = MPI_Wtime();
      printDist();// printout again the new array with shortest paths
      printf("total time %f \n",t2-t1);
    }
    /* workers code*/
    else{

        int i, j, k,t=3;
        int out[t];
        MPI_Recv(&dist,n*n,MPI_INT,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        if(my_rank+1!=num_procs)
            ypoloipo=0;
        for (k = slice*(my_rank-1); k < slice*(my_rank-1)+slice+ypoloipo; ++k)
            for (i = 0; i < n; ++i)
                for (j = 0; j < n; ++j)
                    /* If i and j are different nodes and if
                        the paths between i and k and between
                        k and j exist, do */
                    if ((dist[i][k] * dist[k][j] != 0) && (i != j))
                        /* See if you can't get a shorter path
                            between i and j by interspacing
                            k somewhere along the current
                            path */
                        if ((dist[i][k] + dist[k][j] < dist[i][j]) || (dist[i][j] == 0)){
                            dist[i][j] = dist[i][k] + dist[k][j];
                            out[0]=dist[i][j];
                            out[1]=i;
                            out[2]=j;
                            MPI_Send(&out,t,MPI_INT,MASTER,0,MPI_COMM_WORLD);
                        }
        MPI_Send(0,0,MPI_INT,MASTER,DIETAG,MPI_COMM_WORLD);
    }

    /* Shut down MPI */
    MPI_Finalize();

    return 0;
}
