//#pragma once

#include <mpi.h>
#include <chrono>
#include <stdbool.h>

namespace mpi {
  /* classes */


  class devices {
  public:
    int num_device = 0;
    int *devices;

    ~devices() {
      if (num_device != 0)
        free(devices);
    }
  };

  class topology {
  public:
    int num_node = 0;

    int my_rank;
    int my_rank_sm;
    bool is_device = false;
    int device;

    int *rank_all;
    int *size_sm;
    int *num_device;

    ~topology() {
      free(rank_all);
      free(size_sm);
      free(num_device);
    }
  };


  /*
  utils functions
  */


  topology* get_global_topology(MPI_Comm global_comm, MPI_Comm comm_sm, devices *device) {
    topology* topo = new topology;

    /* read global and local size */
    int my_rank_all, size_all, size_sm;
    MPI_Comm_rank (global_comm, &my_rank_all);
    MPI_Comm_size (global_comm, &size_all);
    MPI_Comm_rank (comm_sm, &topo->my_rank_sm);
    MPI_Comm_size (comm_sm, &size_sm);

    /* get a simple topology list */
    int* global_topology = (int* )malloc(3 * size_all * sizeof(int));
    global_topology[3*my_rank_all] = topo->my_rank_sm;
    global_topology[3*my_rank_all + 1] = size_sm - device->num_device;
    global_topology[3*my_rank_all + 2] = device->num_device;
    MPI_Allgather(&global_topology[3*my_rank_all], 3, MPI_INT, global_topology, 3, MPI_INT, MPI_COMM_WORLD);

    //if (topo->my_rank_sm == 0) {
      /* get the number of shared node */
      int node_size = 0;
      for (int i = 0; i < size_all; i++)
        if (global_topology[3*i] == 0)
          node_size++;

      topo->num_node = node_size;

      /* get the actual topology */
      topo->rank_all = (int* )malloc(node_size * sizeof(int));
      topo->size_sm = (int* )malloc(node_size * sizeof(int));
      topo->num_device = (int* )malloc((node_size--) * sizeof(int));
      for (int i = size_all - 1; i >= 0 ; i--)
        if (global_topology[3*i] == 0) {
          if (i == my_rank_all - topo->my_rank_sm)
            topo->my_rank = node_size;

          topo->rank_all[node_size] = i;
          topo->size_sm[node_size] = global_topology[3*i + 1];
          topo->num_device[node_size] = global_topology[3*i + 2];
          if (node_size-- == 0)
            break;
        }
    //}

    int zero_device_offset = std::max(0, size_sm - device->num_device);
    if(topo->my_rank_sm >= zero_device_offset) {
      topo->is_device = true;
      topo->device = device->devices[topo->my_rank_sm - zero_device_offset];
    }

    /* freeing and return */
    free(global_topology);
    return topo;
  }


  /*
  benchmark functions
  */


  namespace benchmark {
    /* becnhmark size global variable */
    int benchmark_size = 1000000;

    double timeit(void (*func)()) {
      auto start = std::chrono::high_resolution_clock::now();

      //function call
      (*func)();

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      return duration.count();
    }

    double single_thread_benchmark() {
      return benchmark_size / timeit([]() {
        float x = 0.0;
        for (int i = 0; i < benchmark_size; i++) x += i;
      });
    }
  }
}

#ifdef _OPENMP

#include <omp.h>

namespace mpi {
  /* utils functions using openmp */
  namespace utils {
    devices* omp_target_list() {
      devices* device = new devices;
      /* read device numbr */
      device->num_device = omp_get_num_devices();
      device->devices = (int*)malloc(device->num_device * sizeof(int));

      for (int i = 0; i < device->num_device; i++)
        device->devices[i] = i;

      return device;
    }
  }


  /*
  gpu benchmark function using openmp
  */


  namespace benchmark {
    double single_gpu_benchmark() {
      return benchmark_size / timeit([]() {
        float *x = (float *)malloc(benchmark_size * sizeof(float));
        #pragma omp target teams distribute parallel for map(tofrom:x[:benchmark_size])
        for (int i = 0; i < benchmark_size; i++) x[i] = i;
      });
    }
  }
}
#endif
