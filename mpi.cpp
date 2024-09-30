#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdint>
#include <utimer.hpp>
#include <utils.hpp>
#include <string>
#include <algorithm>
#include <memory>
#include <mpi.h>
#include <omp.h>

inline double compute_element_diag(std::vector<double> &M, const uint64_t &m, const uint64_t &k, const uint64_t &N) {
    double c = 0;
    #pragma opm parallel for
    for(uint64_t i=0; i<=k; i++){
        c += M[index(m, m+i, N)]*M[index(m+k-i, m+k, N)];
    }
    c = std::cbrt(c);
    return c;
}


void wavefront(std::vector<double> &M, const uint64_t &N, int id_worker, int n_worker) {

    for(uint64_t k = 1; k < N; k++) {

        uint64_t chunk_size = std::ceil(double(N-k)/n_worker);
        std::vector<double> diag_sync = std::vector(chunk_size * n_worker, 0.0);
        std::vector<double> diag_worker;
        diag_worker.reserve(chunk_size);
        

        for(uint64_t m = chunk_size*id_worker; m < std::min(chunk_size*(id_worker+1), N-k); m++) {
            diag_worker.push_back( compute_element_diag(M, m, k, N) );
        }

        MPI_Allgather(diag_worker.data(), chunk_size, MPI_DOUBLE, diag_sync.data(), chunk_size, MPI_DOUBLE, MPI_COMM_WORLD);

        for(uint64_t m=0; m<N-k; m++){
            M[index(m, m+k, N)] = diag_sync[m];
        }


    }
}




int main (int argc, char *argv[]) {


    MPI_Init(&argc, &argv);
    int id_worker, n_worker;
    MPI_Comm_rank(MPI_COMM_WORLD, &id_worker);
    MPI_Comm_size(MPI_COMM_WORLD, &n_worker);

    uint64_t N = 4;  // default size of the matrix
    if(argc > 1){
        N = std::stoull(argv[1]); // input size of the matrix (NxN)
    }
  
    // allocate the matrix
    std::vector<double> M(N*N, 0);

    // initialize the matrix
    for(uint64_t k=0; k<N; k++){
        M[index(k, k, N)] = double(k+1)/N;
    }

    START(timer);
    // call the wavefront function
    wavefront(M, N, id_worker, n_worker);
    STOP(timer, elapsed);

    int eltime = (int)elapsed;
    std::vector<int> elapsed_toprint(n_worker, 0);
    MPI_Gather(&eltime, 1, MPI_INT, elapsed_toprint.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(id_worker==0){
        for(int &eltime : elapsed_toprint){
            std::cout << "Elapsed time: " << eltime << " usec" << std::endl;
            std::cout << "cell:" << M[N-1] << std::endl;
        }
    }
    

    if(N < 10){
        // print the matrix
        print_matrix(M, N);
    }

    MPI_Finalize();

}