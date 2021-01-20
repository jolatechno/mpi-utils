//#pragma once

#include <mpi.h>
#include <chrono>
#include <stdbool.h>

namespace mpi {
  /* enable barier */
  bool enable_barrier = false;



  /*
  classes
  */


  class devices {
  public:
    int num_device = 0;
    int *devices;
  };

  class topology {
  public:
    int num_node = 0;
    int my_rank;
    int *rank_all;
    int *size_sm;
    int *num_device;
  };


  /*
  utils functions
  */


  namespace utils {
    int none_compute(const int, const int, const int, const int) {
      return 0;
    }

    devices none_list_devices() {
      devices dev;
      return dev;
    }

    topology get_global_topology(const int my_rank_sm, const int size_sm, const int num_device) {
      topology topo;

      /* read global size */
      int my_rank_all, size_all;
      MPI_Comm_rank (MPI_COMM_WORLD, &my_rank_all);
      MPI_Comm_size (MPI_COMM_WORLD, &size_all);

      /* get a simple topology list */
      int* global_topology = (int* )malloc(3 * size_all * sizeof(int));
      global_topology[3*my_rank_all] = my_rank_sm;
      global_topology[3*my_rank_all + 1] = size_sm - num_device;
      global_topology[3*my_rank_all + 2] = num_device;
      MPI_Allgather(&global_topology[3*my_rank_all], 3, MPI_INT, global_topology, 3, MPI_INT, MPI_COMM_WORLD);

      if (my_rank_sm == 0) {
        /* get the number of shared node */
        int node_size = 0;
        for (int i = 0; i < size_all; i++)
          if (global_topology[3*i] == 0)
            node_size++;

        topo.num_node = node_size;

        /* get the actual topology */
        topo.rank_all = (int* )malloc(node_size * sizeof(int));
        topo.size_sm = (int* )malloc(node_size * sizeof(int));
        topo.num_device = (int* )malloc((node_size--) * sizeof(int));
        for (int i = size_all - 1; i >= 0 ; i--)
          if (global_topology[3*i] == 0) {
            if (i == my_rank_all)
              topo.my_rank = node_size;

            topo.rank_all[node_size] = i;
            topo.size_sm[node_size] = global_topology[3*i + 1];
            topo.num_device[node_size] = global_topology[3*i + 2];
            if (node_size-- == 0)
              break;
          }
      }

      /* freeing and return */
      free(global_topology);
      return topo;
    }
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


  /*
  gpu and cpu share function
  */


  int cpu_gpu_share(int n_iter, devices (*list_devices)(), int (*mem_update)(const topology&), int (*cpu_compute)(const int, const int, const int, const int), int (*gpu_compute)(const int, const int, const int, const int), int split_type) {
    int err;

    /* initialize variables */
    MPI_Aint local_window_count;
    MPI_Comm comm_sm;
    int my_rank_all, size_all, my_rank_sm, size_sm;

    /* read mpi size */
    MPI_Comm_rank (MPI_COMM_WORLD, &my_rank_all);
    MPI_Comm_size (MPI_COMM_WORLD, &size_all);
    MPI_Comm_split_type (MPI_COMM_WORLD, split_type, 0, MPI_INFO_NULL, &comm_sm);
    MPI_Comm_rank (comm_sm, &my_rank_sm);
    MPI_Comm_size (comm_sm, &size_sm);

    /* get cpu topology */
    devices device = (*list_devices)();
    topology topo = utils::get_global_topology(my_rank_sm, size_sm, device.num_device);

    for (int i = 0; i < n_iter; i++) {
      /* update memory */
      if (my_rank_sm == 0) {
        /* gpu memory */
        err = (*mem_update)(topo);

        /* error handling */
        /* only on the first thread for now */
        if (err != 0)
          return err;
      }

      if (my_rank_sm < device.num_device) {
        /* gpu compute */
        err = (*gpu_compute)(my_rank_all, size_all, device.devices[my_rank_sm], device.num_device);

        /* error handling */
        /* only on the current thread for now */
        if (err != 0)
          return err;
      } else {
        /* cpu compute */
        err = (*cpu_compute)(my_rank_all, size_all, my_rank_sm - device.num_device, size_sm - device.num_device);

        /* error handling */
        /* only on the current thread for now */
        if (err != 0)
          return err;
      }

      /* barier between iterations */
      if (enable_barrier) {
        MPI_Barrier(comm_sm);
        MPI_Barrier(MPI_COMM_WORLD);
      }
    }

    if (device.num_device != 0)
      free(device.devices);

    if (my_rank_sm == 0) {
      free(topo.rank_all);
      free(topo.size_sm);
      free(topo.num_device);
    }

    return 0;
  }


  /*
  sipmple cpu-only share function
  */


  int cpu_share(int n_iter, int (*mem_update)(const topology&), int (*compute)(const int, const int, const int, const int), int split_type) {
    return cpu_gpu_share(n_iter, &utils::none_list_devices, mem_update, compute, &utils::none_compute, split_type);
  }

  int cpu_share(int n_iter, int (*mem_update)(const topology&), int (*compute)(const int, const int, const int, const int)) {
    return cpu_share(n_iter, mem_update, compute, MPI_COMM_TYPE_SHARED);
  }

  int cpu_share(int (*mem_update)(const topology&), int (*compute)(const int, const int, const int, const int)) {
    return cpu_share(1, mem_update, compute);
  }


  /*
  gpu-only share function
  */


  int gpu_share(int n_iter, devices (*list_devices)(), int (*mem_update)(const topology&), int (*gpu_compute)(const int, const int, const int, const int), int split_type) {
    return cpu_gpu_share(n_iter, &utils::none_list_devices, mem_update, &utils::none_compute, gpu_compute, split_type);
  }
}


#ifdef _OPENMP

#include <omp.h>

namespace mpi {
  /* utils functions using openmp */
  namespace utils {
    devices omp_target_list() {
      devices dev;
      /* read device numbr */
      dev.num_device = omp_get_num_devices();
      dev.devices = (int*)malloc(dev.num_device * sizeof(int));

      for (int i = 0; i < dev.num_device; i++)
        dev.devices[i + 1] = i;

      return dev;
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


  /*
  functions using openmp for gpu detection
  */


  int cpu_gpu_share(int n_iter, int (*mem_update)(const topology&), int (*cpu_compute)(const int, const int, const int, const int), int (*gpu_compute)(const int, const int, const int, const int)) {
    return mpi::cpu_gpu_share(n_iter, &utils::omp_target_list, mem_update, cpu_compute, gpu_compute, MPI_COMM_TYPE_SHARED);
  }

  int cpu_gpu_share(int (*mem_update)(const topology&), int (*cpu_compute)(const int, const int, const int, const int), int (*gpu_compute)(const int, const int, const int, const int)) {
    return mpi::cpu_gpu_share(1, mem_update, cpu_compute, gpu_compute);
  }

  int gpu_share(int n_iter, int (*mem_update)(const topology&), int (*gpu_compute)(const int, const int, const int, const int)) {
    return gpu_share(n_iter, &utils::omp_target_list, mem_update, gpu_compute, MPI_COMM_TYPE_SHARED);
  }

  int gpu_share(int (*mem_update)(const topology&), int (*gpu_compute)(const int, const int, const int, const int)) {
    return gpu_share(1, mem_update, gpu_compute);
  }
}
#endif
