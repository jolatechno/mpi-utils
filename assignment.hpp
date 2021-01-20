#include <mpi.h>
#include <stdexcept>

namespace mpi {
  /* global status */
  MPI_Status receive_status;


  /*
  hiden utils functions
  */


  namespace {
    template<typename T>
    int repeat_product(int (*send_recv)(int, int, int, T*), int to_from, int num_dim, int sizes[], int start[], int length[], T *arr, int prod) {
      int err, i;
      if (num_dim == 1) return send_recv(to_from, start[0], length[0], arr);

      int next_prod = prod / sizes[1];

      for (i = start[0]; i < start[0] + length[0]; i++) {
        err = repeat_product(send_recv, to_from, num_dim - 1, sizes + 1, start + 1, length + 1, arr + prod*i, next_prod);
        if (err = 0) return err;
      }

      return 0;
    }

    template<typename T>
    int repeat_no_product(int (*send_recv)(int, int, int, T*), int to_from, int num_dim, int sizes[], int start[], int length[], T *arr) {
      if (num_dim == 1) return send_recv(to_from, start[0], length[0], arr);

      int prod = sizes[1];
      for (int i = 2; i < num_dim; i++) prod *= sizes[i];

      return repeat_product(send_recv, to_from, num_dim, sizes, start, length, arr, prod);
    }
  }


  /*
  sender
  */


  template<typename T>
  int send(int to, int start, int length, T *arr) { throw std::runtime_error("Unknown type in send"); return -1; }
  int send(int to, int start, int length, char *arr) { return MPI_Send(arr + start, length, MPI_CHAR, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, unsigned char *arr) { return MPI_Send(arr + start, length, MPI_UNSIGNED_CHAR, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, short *arr) { return MPI_Send(arr + start, length, MPI_SHORT, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, unsigned short *arr) { return MPI_Send(arr + start, length, MPI_UNSIGNED_SHORT, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, int *arr) { return MPI_Send(arr + start, length, MPI_INT, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, unsigned int *arr) { return MPI_Send(arr + start, length, MPI_UNSIGNED, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, long int *arr) { return MPI_Send(arr + start, length, MPI_LONG, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, unsigned long int *arr) { return MPI_Send(arr + start, length, MPI_UNSIGNED_LONG, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, long long int *arr) { return MPI_Send(arr + start, length, MPI_LONG_LONG_INT, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, float *arr) { return MPI_Send(arr + start, length, MPI_FLOAT, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, double *arr) { return MPI_Send(arr + start, length, MPI_DOUBLE, to, 1, MPI_COMM_WORLD); }
  int send(int to, int start, int length, long double *arr) { return MPI_Send(arr + start, length, MPI_LONG_DOUBLE, to, 1, MPI_COMM_WORLD); }

  template<typename T, size_t NDIM>
  int send(int to, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr) {
    return repeat_no_product(send, to, NDIM, sizes, start, length, arr);
  }


  /*
  receiver
  */


  template<typename T>
  int receive(int from, int start, int length, T *arr) { throw std::runtime_error("Unknown type in receive"); return -1; }
  int receive(int from, int start, int length, char *arr) { return MPI_Recv(arr + start, length, MPI_CHAR, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, unsigned char *arr) { return MPI_Recv(arr + start, length, MPI_UNSIGNED_CHAR, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, short *arr) { return MPI_Recv(arr + start, length, MPI_SHORT, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, unsigned short *arr) { return MPI_Recv(arr + start, length, MPI_UNSIGNED_SHORT, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, int *arr) { return MPI_Recv(arr + start, length, MPI_INT, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, unsigned int *arr) { return MPI_Recv(arr + start, length, MPI_UNSIGNED, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, long int *arr) { return MPI_Recv(arr + start, length, MPI_LONG, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, unsigned long int *arr) { return MPI_Recv(arr + start, length, MPI_UNSIGNED_LONG, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, long long int *arr) { return MPI_Recv(arr + start, length, MPI_LONG_LONG_INT, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, float *arr) { return MPI_Recv(arr + start, length, MPI_FLOAT, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, double *arr) { return MPI_Recv(arr + start, length, MPI_DOUBLE, from, 1, MPI_COMM_WORLD, &receive_status); }
  int receive(int from, int start, int length, long double *arr) { return MPI_Recv(arr + start, length, MPI_LONG_DOUBLE, from, 1, MPI_COMM_WORLD, &receive_status); }

  template<typename T, size_t NDIM>
  int receive(int from, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr) {
    return repeat_no_product(receive, from, NDIM, sizes, start, length, arr);
  }

}

#ifdef _OPENMP

#include <omp.h>

namespace mpi {
  /* omp sender */
  template<typename T>
  int omp_send(int dev, int to, int start, int length, T *arr) {
    /* copy from target */
    T *buff = (T*)malloc(length * sizeof(T));
    int err = omp_target_memcpy(buff, arr, length, 0, start, omp_get_initial_device(), dev);
    /* error management */
    if (err != 0) {
      free(buff);
      return err;
    }

    /* send */
    err = send(to, start, length, buff);

    /* free and return */
    free(buff);
    return err;
  }

  template<typename T, size_t NDIM>
  int omp_send(int to, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr) {
    return repeat_no_product(omp_send, to, NDIM, sizes, start, length, arr);
  }


  /*
  omp receiver
  */


  template<typename T>
  int omp_receive(int dev, int from, int start, int length, T *arr) {
    /* cpu buffer */
    T *buff = (T*)malloc(length * sizeof(T));

    /* receive to buffer */
    int err = receive(from, start, length, buff);
    /* error management */
    if (err != 0) {
      free(buff);
      return err;
    }

    /* copy to target */
    err = omp_target_memcpy(arr, buff, length, start, 0, dev, omp_get_initial_device());

    /* free and return */
    free(buff);
    return err;
  }

  template<typename T, size_t NDIM>
  int omp_receive(int from, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr) {
    return repeat_no_product(omp_receive, from, NDIM, sizes, start, length, arr);
  }
}
#endif
