//#pragma once

#include <mpi.h>
#include <chrono>
#include <stdbool.h>

namespace mpi {
  /* enable barier */
  bool enable_barrier = false;


  /*
  utils functions
  */


  namespace utils {
    int none_mem_update(int, int, int) {
      return 0;
    };

    int none_compute(int, int, int, int) {
      return 0;
    };

    int* none_list_devices() {
      int *list = (int*)calloc(1, sizeof(int));
      return list;
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


  int cpu_gpu_share(int n_iter, int* (*list_devices)(), int (*mem_update_cpu)(int, int, int), int (*mem_update_gpu)(int, int, int), int (*cpu_compute)(int, int, int, int), int (*gpu_compute)(int, int, int, int), int split_type) {
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

    /* list device */
    int *devices = (*list_devices)();
    int num_device = *(devices++);

    for (int i = 0; i < n_iter; i++) {
      /* update memory */
      if (my_rank_sm == 0) {
        /* gpu memory */
        err = (*mem_update_gpu)(my_rank_all, size_all, num_device);

        /* error handling */
        /* only on the first thread for now */
        if (err != 0)
          return err;

        /* cpu memory */
        err = (*mem_update_cpu)(my_rank_all, size_all, size_sm - num_device);

        /* error handling */
        /* only on the first thread for now */
        if (err != 0)
          return err;
      }

      if (my_rank_sm < num_device) {
        /* gpu compute */
        err = (*gpu_compute)(my_rank_all, size_all, devices[my_rank_sm], num_device);

        /* error handling */
        /* only on the current thread for now */
        if (err != 0)
          return err;
      } else {
        /* cpu compute */
        err = (*cpu_compute)(my_rank_all, size_all, my_rank_sm - num_device, size_sm - num_device);

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

    return 0;
  }


  /*
  sipmple cpu-only share function
  */


  int cpu_share(int n_iter, int (*mem_update)(int, int, int), int (*compute)(int, int, int, int), int split_type) {
    return cpu_gpu_share(n_iter, &utils::none_list_devices, mem_update, &utils::none_mem_update, compute, &utils::none_compute, split_type);
  }

  int cpu_share(int n_iter, int (*mem_update)(int, int, int), int (*compute)(int, int, int, int)) {
    return cpu_share(n_iter, mem_update, compute, MPI_COMM_TYPE_SHARED);
  }

  int cpu_share(int (*mem_update)(int, int, int), int (*compute)(int, int, int, int)) {
    return cpu_share(1, mem_update, compute);
  }


  /*
  gpu-only share function
  */


  int gpu_share(int n_iter, int* (*list_devices)(), int (*mem_update_gpu)(int, int, int), int (*gpu_compute)(int, int, int, int), int split_type) {
    return cpu_gpu_share(n_iter, &utils::none_list_devices, &utils::none_mem_update, mem_update_gpu, &utils::none_compute, gpu_compute, split_type);
  }
}


#ifdef _OPENMP

#include <omp.h>

namespace mpi {
  /* utils functions using openmp */
  namespace utils {
    int* omp_target_list() {
      /* read device numbr */
      const int num_device = omp_get_num_devices();

      int *devices = (int*)malloc((num_device + 1) * sizeof(int));

      devices[0] = num_device;
      for (int i = 0; i < num_device; i++)
        devices[i + 1] = i;

      return devices;
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


  int cpu_gpu_share(int n_iter, int (*mem_update_cpu)(int, int, int), int (*mem_update_gpu)(int, int, int), int (*cpu_compute)(int, int, int, int), int (*gpu_compute)(int, int, int, int)) {
    return mpi::cpu_gpu_share(n_iter, &utils::omp_target_list, mem_update_cpu, mem_update_gpu, cpu_compute, gpu_compute, MPI_COMM_TYPE_SHARED);
  }

  int cpu_gpu_share(int (*mem_update_cpu)(int, int, int), int (*mem_update_gpu)(int, int, int), int (*cpu_compute)(int, int, int, int), int (*gpu_compute)(int, int, int, int)) {
    return mpi::cpu_gpu_share(1, mem_update_cpu, mem_update_gpu, cpu_compute, gpu_compute);
  }

  int gpu_share(int n_iter, int (*mem_update)(int, int, int), int (*gpu_compute)(int, int, int, int)) {
    return gpu_share(n_iter, &utils::omp_target_list, mem_update, gpu_compute, MPI_COMM_TYPE_SHARED);
  }

  int gpu_share(int (*mem_update)(int, int, int), int (*gpu_compute)(int, int, int, int)) {
    return gpu_share(1, mem_update, gpu_compute);
  }
}
#endif
