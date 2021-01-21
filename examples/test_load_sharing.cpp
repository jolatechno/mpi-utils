#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "../load_sharing.hpp"

int main(int argc, char** argv) {
  // Initialisation
  int err = MPI_Init(&argc, &argv); if (err != 0) return err;

  /* split communicator size */
  MPI_Comm comm_sm;
  /*
  MPI_Comm_split_type (MPI_COMM_WORLD, OMPI_COMM_TYPE_SOCKET, 0, MPI_INFO_NULL, &comm_sm);

  hack for my two 4-core computers :
  */
  int my_rank; MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int group[4] = {0, 1, 2, 3};
  if (my_rank >= 4)
    for (int i = 0; i < 4; i++)
      group[i] += 4;


  MPI_Group world_group, node_group;
  MPI_Comm_group(MPI_COMM_WORLD, &world_group);
  MPI_Group_incl(world_group, 4, group, &node_group);
  MPI_Comm_create_group(MPI_COMM_WORLD, node_group, 0, &comm_sm);
  /*
  end of the hack
  */


  mpi::benchmark::benchmark_size *= 500;

  mpi::devices* dev = new mpi::devices;
  mpi::topology* topo = mpi::get_global_topology(MPI_COMM_WORLD, comm_sm, dev);

  MPI_Barrier(MPI_COMM_WORLD);
  if ( topo->my_rank == 0 && topo->my_rank_sm == 0) printf("\n");
  MPI_Barrier(MPI_COMM_WORLD);

  if(topo->my_rank_sm == 0)
    printf("%d/%d(node), %d cpu and %d gpu\n", topo->my_rank + 1, topo->num_node, topo->size_sm[topo->my_rank], topo->num_device[topo->my_rank]);

  MPI_Barrier(MPI_COMM_WORLD);
  if ( topo->my_rank == 0 && topo->my_rank_sm == 0) printf("\n");
  MPI_Barrier(MPI_COMM_WORLD);

  if(topo->is_device) {
    printf("%d/%d(node), %d/%d(gpu): %f iter/ms\n", topo->my_rank + 1, topo->num_node, topo->device, topo->num_device[topo->my_rank], mpi::benchmark::single_gpu_benchmark());
  } else {
    printf("%d/%d(node), %d/%d(cpu): %f iter/ms\n", topo->my_rank + 1, topo->num_node, topo->my_rank_sm + 1, topo->size_sm[topo->my_rank], mpi::benchmark::single_thread_benchmark());
  }

#ifdef _OPENMP
  dev = mpi::utils::omp_target_list();
  topo = mpi::get_global_topology(MPI_COMM_WORLD, comm_sm, dev);

  MPI_Barrier(MPI_COMM_WORLD);
  if ( topo->my_rank == 0 && topo->my_rank_sm == 0) printf("\n");
  MPI_Barrier(MPI_COMM_WORLD);

  if(topo->my_rank_sm == 0)
    printf("%d/%d(node), %d cpu and %d gpu\n", topo->my_rank + 1, topo->num_node, topo->size_sm[topo->my_rank], topo->num_device[topo->my_rank]);

  MPI_Barrier(MPI_COMM_WORLD);
  if ( topo->my_rank == 0 && topo->my_rank_sm == 0) printf("\n");
  MPI_Barrier(MPI_COMM_WORLD);

  if(topo->is_device) {
    printf("%d/%d(node), %d/%d(gpu): %f iter/ms\n", topo->my_rank + 1, topo->num_node, topo->device, topo->num_device[topo->my_rank], mpi::benchmark::single_gpu_benchmark());
  } else {
    printf("%d/%d(node), %d/%d(cpu): %f iter/ms\n", topo->my_rank + 1, topo->num_node, topo->my_rank_sm + 1, topo->size_sm[topo->my_rank], mpi::benchmark::single_thread_benchmark());
  }

#endif

  // Finalisation
  return MPI_Finalize();
}
