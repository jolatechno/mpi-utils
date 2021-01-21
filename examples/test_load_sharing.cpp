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
  MPI_Comm_split_type (MPI_COMM_WORLD, OMPI_COMM_TYPE_CORE, 0, MPI_INFO_NULL, &comm_sm);

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

  printf("%d/%d(node), %d/%d(local) %s\n", topo->my_rank + 1, topo->num_node, topo->my_rank_sm + 1, topo->size_sm[topo->my_rank], topo->is_device ? "gpu": "cpu");

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
    printf("%d/%d(node), %d/%d(gpu)\n", topo->my_rank + 1, topo->num_node, topo->device, topo->num_device[topo->my_rank]);
  } else {
    printf("%d/%d(node), %d/%d(cpu)\n", topo->my_rank + 1, topo->num_node, topo->my_rank_sm + 1, topo->size_sm[topo->my_rank]);
  }

#endif

  // Finalisation
  return MPI_Finalize();
}
