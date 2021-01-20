#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "../assignment.hpp"

int main(int argc, char** argv) {
    // Initialisation
    int err = MPI_Init(&argc, &argv); if (err != 0) return err;

    // Reading size and rank
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //testing
    double mat[3][5][5];
    int sizes[3] = {3, 5, 5};
    int length[3] = {2, 4, 3}; //length of the block to copy

    if (rank == 0) {

      //initializing matrix
      for (int k = 0; k < 3; k++)
        for (int i = 0; i < 5; i++)
          for (int j = 0; j < 5; j++)
            mat[k][i][j] = (double)k + (double)i/10 + (double)j/100;

      //printing matrix
      for (int k = 0; k < 3; k++) {
        for (int i = 0; i < 5; i++) {
          for (int j = 0; j < 5; j++)
            printf("%6.2f   ", mat[k][i][j]);
          printf ("\n");
        }
        printf ("\n");
      }
      printf ("\n--------\n\n\n");

      //sending matrix
      int start[3] = {0, 0, 2};

      err = mpi::send(1, sizes, start, length, &mat[0][0][0]); if (err != 0) return err;
    } else if (rank == 1) {

      //receving matrix
      int start[3] = {1, 1, 0};

      err = mpi::receive(0, sizes, start, length, &mat[0][0][0]); if (err != 0) return err;

      //printing matrix
      for (int k = 0; k < 3; k++) {
        for (int i = 0; i < 5; i++) {
          for (int j = 0; j < 5; j++)
            printf("%6.2f   ", mat[k][i][j]);
          printf ("\n");
        }
        printf ("\n");
      }
    }

    // Finalisation
    return MPI_Finalize();
}
