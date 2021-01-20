# mpi-utils

This is a simple repository containing header file that provide utility functions for __MPI__ that I use in my HPC projects.

The [assignment.hpp](./assignment.hpp) file contains functions made to send blocks of multi-dimensional arrays, with varying sizes, and with the offsets (in each dimensions) from the sending node able to be different from the offsets of the receiving node. This is perfect for simulations, where a fractured space can have different indexes on different nodes.

The [load_sharing.hpp](./load_sharing.hpp) file contains functions made to enable automatic load-sharing using a hybrid __MPI__ model (__MPI__ nodes with shared memory), supporting cpu-only functions, gpu-only and cpu/gpu hybrid functions.

## Compilation

You can use `mpic++` to compile the provided [examples](./examples), and to run them using `mpirun`.

## Usage

### assignment

#### available functions

To use the send and receive functions provided by this repository, you need to import the [assignment.hpp](./assignment.hpp) file.

```cpp
int mpi::send(int to, int start, int length, T *arr)
int mpi::send(int to, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr)
int mpi::receive(int from, int start, int length, T *arr)
int mpi::receive(int from, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr)

/* only if compiled with openmp */
int mpi::omp_send(int dev, int to, int start, int length, T *arr)
int mpi::omp_send(int dev, int to, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr)
int mpi::omp_receive(int dev, int from, int start, int length, T *arr)
int mpi::omp_receive(int dev, int from, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr)
```

#### example

You can see an example code in the [examples/test_assignment.cpp](./examples/test_assignment.cpp) file, which is explained bellow:

```cpp
//testing
double mat[3][5][5];
int sizes[3] = {3, 5, 5};
int length[3] = {2, 4, 3}; //length of the block to copy
```

We first create a new 3d array, and define its size and the length of the block to copy.

We then fill it with recognizable values (see [Results](#results)), and print it.

We then decide the offsets of the block that will be sent, and send it to the other node :

```cpp
//sending matrix
int start[3] = {0, 0, 2};

err = mpi::send(1, sizes, start, length, &mat[0][0][0]); if (err != 0) return err;
```

And we decide the offsets at which the block will be received, and receive it from the other node :

```cpp
if (rank == 1) {
  //receving matrix
  int start[3] = {1, 1, 0};

  err = mpi::receive(0, sizes, start, length, &mat[0][0][0]); if (err != 0) return err;

  //print the matrix
}
```

We can see that the program worked as intended (see [Results](#results)), copying a block of the right size, with the right offsets both at the receiving end and at the sending end.

#### CUDA (and gpu) aware

You can send directly to a __CUDA__ device (leveraging cuda-aware __MPI__) by using the same function with a device buffer (generated by `cudaMalloc` or by __OPENMP__ or __OPENACC__ inside a target section).

You can also send to any __OPENMP__ device by using the same function with a `omp_` prefix, and by adding as a first argument the `device_number` of the device from which to send or receive.

You might prefer implementing your own __OPENMP__ send and receive function for more special workcases, as those provided by this repository allocate and free cpu buffers as intermediary.

### load sharing

#### classes

#### available functions

To use the automated load-sharing functions provided by this repository, you need to import the [load_sharing.hpp](./load_sharing.hpp) file.

```cpp
int mpi::cpu_gpu_share(int n_iter, int* (*list_devices)(), int (*mem_update_cpu)(int, int, int), int (*mem_update_gpu)(int, int, int), int (*cpu_compute)(int, int, int, int), int (*gpu_compute)(int, int, int, int), int split_type)
int mpi::cpu_share(int n_iter, int (*mem_update)(int, int, int), int (*compute)(int, int, int, int), int split_type)
int mpi::cpu_share(int n_iter, int (*mem_update)(int, int, int), int (*compute)(int, int, int, int))
int mpi::cpu_share(int (*mem_update)(int, int, int), int (*compute)(int, int, int, int))
int mpi::gpu_share(int n_iter, int* (*list_devices)(), int (*mem_update_gpu)(int, int, int), int (*gpu_compute)(int, int, int, int), int split_type)

int mpi::utils::none_mem_update(int, int, int)
int mpi::utils::none_compute(int, int, int, int)
int* mpi::utils::none_list_devices()

double mpi::benchmark::timeit(void (*func)())
double mpi::benchmark::single_thread_benchmark()

/* only if compiled with openmp */
int mpi::cpu_gpu_share(int n_iter, int (*mem_update_cpu)(int, int, int), int (*mem_update_gpu)(int, int, int), int (*cpu_compute)(int, int, int, int), int (*gpu_compute)(int, int, int, int))
int mpi::cpu_gpu_share(int (*mem_update_cpu)(int, int, int), int (*mem_update_gpu)(int, int, int), int (*cpu_compute)(int, int, int, int), int (*gpu_compute)(int, int, int, int))
int mpi::gpu_share(int n_iter, int (*mem_update)(int, int, int), int (*gpu_compute)(int, int, int, int))
int mpi::gpu_share(int (*mem_update)(int, int, int), int (*gpu_compute)(int, int, int, int))

int* mpi::utils::omp_target_list()

double mpi::benchmark::single_gpu_benchmark()
```

Functions pointer passed as argument obey the following rules :

```cpp
int* list_devices()
```

A `list_devices` function is the function that list all device number (as defined by __OPENMP__ for example). It returns a list of `int`, with the first element being the number of device.

```cpp
int mem_update(int my_rank_all, int size_all, int num_device_or_thread)
```

A `mem_update` function is called by the node of local index 0 inside the local communicator, and takes as argument the global rank (`rank_all`) of the node, the size of the global communicator (`size_all`) and the number of local thread (`num_device_or_thread`) or of local devices for gpu.

```cpp
int compute(int my_rank_all, int size_all, int device_index_or_local_rank, int num_device_or_thread)
```

A `compute` function is called by each compute node (or each device), and takes as argument the global rank (`rank_all`) of the node, the size of the global communicator (`size_all`), the index inside the local communicator (`device_index_or_local_rank`) or of the device, and the number of local thread (`num_device_or_thread`) or of local devices for gpu.

`mem_update` and `compute` are called in this order for each iterations.

The `split_type` argument is passed to the [MPI_Comm_split_type](https://www.open-mpi.org/doc/v3.1/man3/MPI_Comm_split_type.3.php) functions which specify how the global communicator will be split.

#### example

You can see an example code in the [examples/test_load_sharing.cpp](./examples/test_load_sharing.cpp) file.

# Results

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
