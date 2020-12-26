#include <mpi.h>
#include <stdexcept>

namespace mpi {
  /*
  global variable
  */


  MPI_Status receive_status;


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

  namespace { //hiden recursive function
    template<typename T>
    int send_rec(int from, int num_dim, int sizes[], int start[], int length[], T *arr, int prod) {
      int err, i;
      if (num_dim == 1) return send(from, start[0], length[0], arr);

      int next_prod = prod / sizes[1];

      for (i = start[0]; i < start[0] + length[0]; i++) {
        err = send_rec(from, num_dim - 1, sizes + 1, start + 1, length + 1, arr + prod*i, next_prod);
        if (err = 0) return err;
      }

      return 0;
    }
  }

  template<typename T, size_t NDIM>
  int send(int from, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr) {
    if (NDIM == 1) return send(from, start[0], length[0], arr);

    int prod = sizes[1];
    for (int i = 2; i < NDIM; i++) prod *= sizes[i];

    return send_rec(from, NDIM, sizes, start, length, arr, prod);
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

  namespace { //hiden recursive function
    template<typename T>
    int receive_rec(int from, int num_dim, int sizes[], int start[], int length[], T *arr, int prod) {
      int err, i;
      if (num_dim == 1) return receive(from, start[0], length[0], arr);

      int next_prod = prod / sizes[1];

      for (i = start[0]; i < start[0] + length[0]; i++) {
        err = receive_rec(from, num_dim - 1, sizes + 1, start + 1, length + 1, arr + prod*i, next_prod);
        if (err = 0) return err;
      }

      return 0;
    }
  }

  template<typename T, size_t NDIM>
  int receive(int from, int (&sizes)[NDIM], int (&start)[NDIM], int (&length)[NDIM], T *arr) {
    int err, i;
    if (NDIM == 1) return receive(from, start[0], length[0], arr);

    int prod = sizes[1];
    for (i = 2; i < NDIM; i++) prod *= sizes[i];

    return receive_rec(from, NDIM, sizes, start, length, arr, prod);
  }

}
