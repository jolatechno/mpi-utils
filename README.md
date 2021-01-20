# mpi-utils

This is a simple repo containing header file that provide utility functions for MPI that I use in my HPC projects.

The [assignment.hpp](./assignment.hpp) file contains function to send blocks of multi-dimensional arrays, with varying sizes, and with the receiving indexes able to be different from the receiving indexes. This is perfect for simulations, where a fractured space can have different indexes on different nodes.

## Compilation

To compile this example, you can use `mpic++` and to run it using `mpirun`.

## Usage

### assignment

To use the send and receive function provided by this directory, you need to import the [assignment.hpp](./assignment.hpp) file.

You can see an example code in the [test_assignment.cpp](./test_assignment.cpp) file, that is explained bellow:

```cpp
// Initialisation
int err = MPI_Init(&argc, &argv); if (err != 0) return err;
```

We just iniatized the MPI comm, returning an error if it fails.

```cpp
//testing
double mat[3][5][5];
int sizes[3] = {3, 5, 5};
int length[3] = {2, 4, 3}; //length of the block to copy
```

We know create a new 3d array, and define its size and the length of the block to copy.

```cpp
if (rank == 0) {

  //initializing matrix
  for (int k = 0; k < 3; k++)
    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 5; j++)
        mat[k][i][j] = (double)k + (double)i/10 + (double)j/100;

  //print the matrix
  //send it
}
```

We know fill it with recognizable values (see [Results](#results)), and print it.

We now decide the starting point of the block that will be sent, and send it to the other node :

```cpp
  //sending matrix
  int start[3] = {0, 0, 2};

  err = mpi::send(1, sizes, start, length, &mat[0][0][0]); if (err != 0) return err;
```

And we decide where the block will be received, and receive it from the other node :

```cpp
if (rank == 1) {
  //receving matrix
  int start[3] = {1, 1, 0};

  err = mpi::receive(0, sizes, start, length, &mat[0][0][0]); if (err != 0) return err;

  //print the matrix
}
```

We then print it and finalize the MPI comm.

We can see that the program worked as intended (see [Results](#results)), exiting without errors, and copying a block of the right size, with the right starting point both at the receiving end and at the sending end.

#### CUDA (and gpu) aware

You can send directly to a __CUDA__ device (leveraging cuda-aware __MPI__) by using the same function with a `cuda_` prefix beforehand.

You can also send to any __OPENMP__ device by using the same function with a `omp_` prefix.

You might prefer implementing your own __OPENMP__ send and receive function for more spetial workcase as those provided by this repository allocate and free cpu buffer as intermediary.

- [ ] TODO

### load sharing

To use the automated load-sharing functions provided by this directory, you need to import the [load_sharing.hpp](./load_sharing.hpp) file.

You can see an example code in the [test_load_sharing.cpp](./test_load_sharing.cpp) file, that is explained bellow:

- [ ] TODO

# Results

### assignment

The first matrix print returned :

```shell
  0.00     0.01     0.02     0.03     0.04   
  0.10     0.11     0.12     0.13     0.14   
  0.20     0.21     0.22     0.23     0.24   
  0.30     0.31     0.32     0.33     0.34   
  0.40     0.41     0.42     0.43     0.44   

  1.00     1.01     1.02     1.03     1.04   
  1.10     1.11     1.12     1.13     1.14   
  1.20     1.21     1.22     1.23     1.24   
  1.30     1.31     1.32     1.33     1.34   
  1.40     1.41     1.42     1.43     1.44   

  2.00     2.01     2.02     2.03     2.04   
  2.10     2.11     2.12     2.13     2.14   
  2.20     2.21     2.22     2.23     2.24   
  2.30     2.31     2.32     2.33     2.34   
  2.40     2.41     2.42     2.43     2.44
```

And the second print returned :

```shell
  0.00     0.00     0.00     0.00     0.00   
  0.00     0.00     0.00     0.00     0.00   
  0.00     0.00     0.00     0.00     0.00   
  0.00     0.00     0.00     0.00     0.00   
  0.00     0.00     0.00     0.00     0.00   

  0.00     0.00     0.00     0.00     0.00   
  0.02     0.03     0.04     0.00     0.00   
  0.12     0.13     0.14     0.00     0.00   
  0.22     0.23     0.24     0.00     0.00   
  0.32     0.33     0.34     0.00     0.00   

  0.00     0.00     0.00     0.00     0.00   
  1.02     1.03     1.04     0.00     0.00   
  1.12     1.13     1.14     0.00     0.00   
  1.22     1.23     1.24     0.00     0.00   
  1.32     1.33     1.34     0.00     0.00
```

### load sharing
