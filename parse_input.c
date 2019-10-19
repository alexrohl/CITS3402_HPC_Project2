// C program for reading
// struct from a file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<omp.h>
#include <mpi.h>
#include<time.h>
#define INFINITY 100000000

#define MAX_LINE_LEN 256
struct MatrixContainer {
  int size;
  int * matrix;
  double time;
};

typedef struct MatrixContainer MatrixContainer;

MatrixContainer get_Matrix(char *filename, int size)
{
    double t = MPI_Wtime();
    MatrixContainer Result;
    //double t = omp_get_wtime();
    //build rows
    int *matrix = malloc(size*size*sizeof(int));
    //matrix = malloc(sizeof(int)*size);


    FILE *fp = fopen(filename, "rb"); /* should check the result */
    char type[MAX_LINE_LEN];
    int i_elem,i,j;

    /*read in size of matrix*/
    fread(&i_elem, sizeof(int), 1, fp);    /* already know this... */
    printf("SIZE %d \n",i_elem);

    for (i=0; i<size; i++) {
      for (j=0; j<size; j++) {

        /*read element*/
        //fscanf(fp,"%d", &i_elem);
        fread(&i_elem, sizeof(int), 1, fp);
        //printf("%d ",i_elem);
        //diagonal values will be zero
        if (i==j) {
          i_elem = 0;

        //unreachable values set distance to infinity
        } else if (i_elem == 0) {
          i_elem = INFINITY;
        }

        matrix[i*size + j] = i_elem;
      }
    }
    Result.size = size;
    Result.matrix = matrix;
    double time_taken = MPI_Wtime() - t;
    Result.time = time_taken;
    return Result;
}

/*
int main()
{
    int r = 3, c = 4, i, j, count;

    int *arr[r];
    for (i=0; i<r; i++)
         arr[i] = (int *)malloc(c * sizeof(int));

    // Note that arr[i][j] is same as *(*(arr+i)+j)
    count = 0;
    for (i = 0; i <  r; i++)
      for (j = 0; j < c; j++)
         arr[i][j] = ++count; // Or *(*(arr+i)+j) = ++count

    for (i = 0; i <  r; i++)
      for (j = 0; j < c; j++)
         printf("%d ", arr[i][j]);

    // Code for further processing and free the   dynamically allocated memory

   return 0;
} */

/*





    int *temp,*row_indices,*column_indices;

    //BUILD row_indices = [0, 1, 3, 3] the number of elements in each row
    row_indices = malloc(sizeof(int));
    //BUILD column_indices: Stores the column index of each non-zero element.
    column_indices = malloc(sizeof(int));

    int *values;
    // values: The non-zero values stored in row-major order
    values = malloc(sizeof(int));

    double *double_values;
    double *double_temp;
    // values: The non-zero values stored in row-major order
    double_values = malloc(sizeof(double));

    int non_zero_counter = 0;

    int i;
    int j;

    //FOR INTEGERS

    if (isInt) {
      int i_elem;

      for (i=0; i<n; i++) {
        for (j=0; j<m; j++) {
          //read element
          fscanf(fp,"%d", &i_elem);

          if (i_elem != 0) {
            //append to values
            values = append_int_to_array(temp, values, i_elem, non_zero_counter);
            row_indices = append_int_to_array(temp, row_indices, i, non_zero_counter);
            column_indices = append_int_to_array(temp, column_indices, j, non_zero_counter);
            non_zero_counter++;
          }
        }
      }
      parsed_matrix.values = values;
      //print_int_array(parsed_matrix.values, non_zero_counter, "values");
    } else {
      //FOR doubleS
      double f_elem;

      for (i=0; i<n; i++) {
        for (j=0; j<m; j++) {
          // i is our row index
          // j is our column index

          //read element
          fscanf(fp,"%lf", &f_elem);

          if (f_elem != 0) {
            //append to values
            double_values = append_double_to_array(double_temp, double_values, f_elem, non_zero_counter);
            row_indices = append_int_to_array(temp, row_indices, i, non_zero_counter);
            column_indices = append_int_to_array(temp, column_indices, j, non_zero_counter);
            non_zero_counter++;

          }
        }
      }
      parsed_matrix.values = double_values;
      //print_double_array(parsed_matrix.values, non_zero_counter, "values");
    }

    parsed_matrix.lenvalues = non_zero_counter;
    parsed_matrix.row_indices = row_indices;
    parsed_matrix.column_indices = column_indices;

    //print_int_array(parsed_matrix.column_indices , non_zero_counter, "column");
    //print_int_array(parsed_matrix.row_indices, non_zero_counter, "rows");

    // Free the pointers
    // free(values);
    //free(column_indices);
    //free(row_indices);

    double time_taken = omp_get_wtime() - t;
    parsed_matrix.time = time_taken;
    printf("Time: %f\n",parsed_matrix.time);
    return parsed_matrix;

}*/
