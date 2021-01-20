#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "../load_sharing.hpp"

int main(int argc, char** argv) {
  int rank_all, err = 0;
  // Initialisation
  err = MPI_Init(&argc, &argv); if (err != 0) return err;
  MPI_Comm_rank (MPI_COMM_WORLD, &rank_all);

  mpi::enable_barier = true;

  mpi::cpu_share(1, mpi::utils::none_mem_update,
  [](int i, int j, int k, int r) {
    printf("node %d/%d(global) %d/%d(local)\n", i + 1, j, k + 1, r);
    return 0;
  },
  OMPI_COMM_TYPE_CORE
  );

  mpi::benchmark::benchmark_size *= 100;

  mpi::cpu_share(1, mpi::utils::none_mem_update,
  [](int i, int j, int k, int r) {
    printf("cpu-only benchmark: %d/%d(global) %f iters/ms\n", i + 1, j, mpi::benchmark::single_thread_benchmark());
    return 0;
  },
  OMPI_COMM_TYPE_CORE
  );

#ifdef _OPENMP

  mpi::cpu_gpu_share(1, mpi::utils::omp_target_list, mpi::utils::none_mem_update, mpi::utils::none_mem_update,
  [](int i, int j, int k, int r) {
    printf("hybrid benchmark (cpu): %d/%d(global) %f iters/ms\n", i + 1, j, mpi::benchmark::single_thread_benchmark());
    return 0;
  },
  [](int i, int j, int k, int r) {
    printf("hybrid benchmark (gpu): %d/%d(global) %f iters/ms\n", i + 1, j, mpi::benchmark::single_gpu_benchmark());
    return 0;
  },
  OMPI_COMM_TYPE_CORE
  );
#endif

  // Finalisation
  return MPI_Finalize();
}
