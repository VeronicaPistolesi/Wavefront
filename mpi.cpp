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
    
    for(uint64_t i=0; i<=k; i++){
        c += M[index(m, m+i, N)] * M[index(m+k, m+k-i, N)];
    }
    c = std::cbrt(c);
    return c;
}


void wavefront(std::vector<double> &M, const uint64_t &N, int id_worker, int n_worker) {

    for(uint64_t k = 1; k < N; k++) {

        std::vector<int> recv_counts(n_worker, (N-k)/n_worker), displs(n_worker, 0);
        
        for(int i=0; i<(N-k)%n_worker; i++){
            recv_counts[i] += 1; 
        }

        for (int i = 1; i < n_worker; i++) {
            displs[i] = displs[i-1]+recv_counts[i-1];
        }

        std::vector<double> diag_sync = std::vector(N-k, 0.0);
        std::vector<double> diag_worker;
        diag_worker.reserve(recv_counts[id_worker]);

        #pragma omp parallel
        {
            std::vector<double> diag_worker_private;
            #pragma omp for nowait schedule(static)
            for(uint64_t m = displs[id_worker]; m < displs[id_worker]+recv_counts[id_worker]; m++) {
                diag_worker_private.push_back( compute_element_diag(M, m, k, N) );
            }
            #pragma omp for schedule(static) ordered 
            for(int i=0; i<omp_get_num_threads(); i++){
                #pragma omp ordered 
                diag_worker.insert(diag_worker.end(), diag_worker_private.begin(), diag_worker_private.end());
            }
        }

        MPI_Allgatherv(diag_worker.data(), diag_worker.size(), MPI_DOUBLE, diag_sync.data(), recv_counts.data(), displs.data(), MPI_DOUBLE, MPI_COMM_WORLD);

        for(uint64_t m=0; m<N-k; m++){
            M[index(m, m+k, N)] = diag_sync[m];
            M[index(m+k, m, N)] = diag_sync[m]; 
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