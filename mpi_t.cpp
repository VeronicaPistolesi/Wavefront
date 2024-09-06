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

inline double compute_element_diag_transpose(std::vector<double> &M1, std::vector<double> &M2, const uint64_t &m, const uint64_t &k, const uint64_t &N) {
    double c = 0;
    #pragma opm parallel for
    for(uint64_t i=0; i<=k; i++){
        c += M1[index(m, m+i, N)]*M2[index(N-(m+k)-1, N-(m+k-i)-1, N)];
    }
    c = std::cbrt(c);
    return c;
}

void wavefront(std::vector<double> &M1, std::vector<double> &M2, const uint64_t &N, int id_worker, int n_worker) {


    for(uint64_t k = 1; k < N; k++) {

        uint64_t chunk_size = std::ceil(double(N-k)/n_worker);
        std::vector<double> diag_sync = std::vector(chunk_size * n_worker, 0.0);
        std::vector<double> diag_worker;
        diag_worker.reserve(chunk_size);
        

        for(uint64_t m = chunk_size*id_worker; m < std::min(chunk_size*(id_worker+1), N-k); m++) {
            diag_worker.push_back( compute_element_diag_transpose(M1, M2, m, k, N) );
        }

        MPI_Allgather(diag_worker.data(), chunk_size, MPI_DOUBLE, diag_sync.data(), chunk_size, MPI_DOUBLE, MPI_COMM_WORLD);

        for(uint64_t m=0; m<N-k; m++){
            M1[index(m, m+k, N)] = diag_sync[m];
            M2[index(N-(m+k)-1, N-m-1, N)] = diag_sync[m];
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
    std::vector<double> M1(N*N, 0);
    std::vector<double> M2(N*N, 0);

    // initialize the matrix
    for(uint64_t k=0; k<N; k++){
        M1[index(k, k, N)] = double(k+1)/N;
        M2[index(N-k-1, N-k-1, N)] = double(k+1)/N;
    }

    START(timer);
    // call the wavefront function
    wavefront(M1, M2, N, id_worker, n_worker);
    STOP(timer, elapsed);
    
    int eltime = (int)elapsed;
    std::vector<int> elapsed_toprint(n_worker, 0);
    MPI_Gather(&eltime, 1, MPI_INT, elapsed_toprint.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(id_worker==0){
        for(int &eltime : elapsed_toprint){
            std::cout << "Elapsed time: " << eltime << " usec" << std::endl;
        }
    }

    if(id_worker == 0 && N < 10){
        // print the matrix
        print_matrix(M1, N);
        print_matrix(M2, N);
    }

    MPI_Finalize();

}