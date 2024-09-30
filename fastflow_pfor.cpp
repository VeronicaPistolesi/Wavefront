#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdint>
#include <utimer.hpp>
#include <utils.hpp>
#include <string>
#include <algorithm>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <memory>

void wavefront(std::vector<double> &M, uint64_t N, uint64_t num_workers) {
    ff::ParallelFor pf;
    
    for (uint64_t k = 1; k < N; k++) {

        pf.parallel_for(0, N-k, 1, [&](uint64_t m) {

            compute_element(M, m, k, N);

        }, num_workers);
    }


}
  

int main (int argc, char *argv[]) {

    uint64_t N = 4;  // default size of the matrix
    uint64_t num_workers = 6; // default num workers

    if(argc > 2){
        N = std::stoull(argv[1]); // input size of the matrix (NxN)
        num_workers = std::stoull(argv[2]);
    }
  
    // allocate the matrix
    std::vector<double> M(N*N, 0);

    // initialize the matrix
    for(uint64_t k=0; k<N; k++){
        M[index(k, k, N)] = double(k+1)/N;
    }

    START(timer);
    // call the wavefront function
    wavefront(M, N, num_workers);
    STOP(timer, elapsed);
    std::cout << "Elapsed time: " << elapsed << " usec" << std::endl;
    std::cout << "cell:" << M[N-1] << std::endl;

    if(N < 10){
        // print the matrix
        print_matrix(M, N);
    }

}