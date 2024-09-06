#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>

inline uint64_t index(uint64_t row, uint64_t col, uint64_t N) {
    return row*N + col;
}

inline void compute_element(std::vector<double> &M, const uint64_t &m, const uint64_t &k, const uint64_t &N) {
    double c = 0;
    for(uint64_t i=0; i<=k; i++){
        c += M[index(m, m+i, N)]*M[index(m+k-i, m+k, N)];
    }
    c = std::cbrt(c);
    M[index(m, m+k, N)] = c;
}

inline void compute_element_transpose(std::vector<double> &M1, std::vector<double> &M2, const uint64_t &m, const uint64_t &k, const uint64_t &N) {
    double c = 0;
    for(uint64_t i=0; i<=k; i++){
        c += M1[index(m, m+i, N)]*M2[index(N-(m+k)-1, N-(m+k-i)-1, N)];
    }
    c = std::cbrt(c);
    M1[index(m, m+k, N)] = c;
    M2[index(N-(m+k)-1, N-m-1, N)] = c;

}

void print_matrix(const std::vector<double> &M, const uint64_t &N) {
    for(uint64_t i=0; i<N; i++){
        for(uint64_t j=0; j<N; j++){
            std::cout << std::setw(5) << std::setprecision(2) << M[index(i, j, N)] << " ";
        }
        std::cout << std::endl;
    }
}

