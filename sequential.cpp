#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdint>
#include <utimer.hpp>
#include <utils.hpp>
#include <string>

void wavefront(std::vector<double> &M, const uint64_t &N) {
    for(uint64_t k=1; k<N; k++){
        for(uint64_t m=0; m<N-k; m++){
            compute_element(M, m, k, N);
        }
    }
}

int main (int argc, char *argv[]) {

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
    wavefront(M, N);
    STOP(timer, elapsed);
    std::cout << "Elapsed time: " << elapsed << " usec" << std::endl;
    std::cout << "cell:" << M[N-1] << std::endl;

    if(N < 20){
        // print the matrix
        print_matrix(M, N);
    }

}