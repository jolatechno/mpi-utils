#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "../load_sharing.hpp"

int main(int argc, char** argv) {
  // Initialisation
  int err = MPI_Init(&argc, &argv); if (err != 0) return err;

  mpi::enable_barrier = true;

  mpi::cpu_share(1,
  [](const mpi::topology& topo) {
    printf("%d/%d(node), %d cpu and %d gpu\n", topo.my_rank + 1, topo.num_node, topo.size_sm[topo.my_rank], topo.num_device[topo.my_rank]);
    return 0;
  },
  [](const int i, const int j, const int k, const int r) {
    printf("node %d/%d(global) %d/%d(local)\n", i + 1, j, k + 1, r);
    return 0;
  },
  OMPI_COMM_TYPE_CORE
  );

  mpi::benchmark::benchmark_size *= 100;

  mpi::cpu_share(1,
  [](const mpi::topology& topo) {
    //printf("%d/%d(node), %d cpu and %d gpu\n", topo.my_rank + 1, topo.num_node, topo.size_sm[topo.my_rank], topo.num_device[topo.my_rank]);
    return 0;
  },
  [](const int i, const int j, const int k, const int r) {
    printf("cpu-only benchmark: %d/%d(global) %f iters/ms\n", i + 1, j, mpi::benchmark::single_thread_benchmark());
    return 0;
  },
  OMPI_COMM_TYPE_CORE
  );

#ifdef _OPENMP

  mpi::cpu_gpu_share(1, mpi::utils::omp_target_list,
  [](const mpi::topology& topo) {
    printf("%d/%d(node), %d cpu and %d gpu\n", topo.my_rank + 1, topo.num_node, topo.size_sm[topo.my_rank], topo.num_device[topo.my_rank]);
    return 0;
  },
  [](const int i, const int j, const int k, const int r) {
    printf("hybrid benchmark (cpu): %d/%d(global) %f iters/ms\n", i + 1, j, mpi::benchmark::single_thread_benchmark());
    return 0;
  },
  [](const int i, const int j, const int k, const int r) {
    printf("hybrid benchmark (gpu): %d/%d(global) %f iters/ms\n", i + 1, j, mpi::benchmark::single_gpu_benchmark());
    return 0;
  },
  OMPI_COMM_TYPE_CORE
  );
#endif

  // Finalisation
  return MPI_Finalize();
}
