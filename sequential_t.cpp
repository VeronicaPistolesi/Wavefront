#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdint>
#include <utimer.hpp>
#include <utils.hpp>
#include <string>

void wavefront(std::vector<double> &M1, std::vector<double> &M2, const uint64_t &N) {
    for(uint64_t k=1; k<N; k++){
        for(uint64_t m=0; m<N-k; m++){
            compute_element_transpose(M1, M2, m, k, N);
        }
    }
}

int main (int argc, char *argv[]) {

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
    wavefront(M1, M2, N);
    STOP(timer, elapsed);
    std::cout << "Elapsed time: " << elapsed << " usec" << std::endl;

    if(N < 10){
        // print the matrix
        print_matrix(M1, N);
        print_matrix(M2, N);
    }

}