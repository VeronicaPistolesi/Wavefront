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
#include <utility>

// da provare N/n_worker con + grande in basso e + grande in alto
struct problem {
    std::vector<double> &M1;
    std::vector<double> &M2;
    uint64_t N;
    uint64_t rel_N;
    int n_worker;
    std::vector<uint64_t> &sum_height;
    std::vector<uint64_t> &height;

    problem(
        std::vector<double> &M1,
        std::vector<double> &M2,
        uint64_t N,
        uint64_t rel_N,
        int n_worker,
        std::vector<uint64_t> &sum_height,
        std::vector<uint64_t> &height
    ) : M1(M1), M2(M2), N(N), rel_N(rel_N), n_worker(n_worker), sum_height(sum_height), height(height) {}
};

inline uint64_t sum_height(const problem &problem, int id_worker) {
    return problem.sum_height[id_worker];
}

inline uint64_t height(const problem &problem, int id_worker) {
    auto height = problem.height[id_worker];
    return height;
}

inline uint64_t index_t(const problem &problem, int id_worker) {
    return problem.N-sum_height(problem, id_worker);
}

inline uint64_t leg_size_t(const problem &problem, int id_worker) {
    return height(problem, id_worker);
}

inline std::pair<uint64_t, uint64_t> index_r(const problem &problem, int id_worker, uint64_t iter) {
    auto N = problem.N;
    auto c = N-sum_height(problem, id_worker+iter+1);
    auto r = N-sum_height(problem, id_worker);
    return std::pair(r, c);
}

inline std::pair<uint64_t, uint64_t> size_r(const problem &problem, int id_worker, uint64_t iter) {
    auto c = height(problem, id_worker+iter+1);  
    auto r = height(problem, id_worker);
    return std::pair(r, c);
}

inline uint64_t abs_to_rel_index(const problem &problem, int id_worker, std::pair<uint64_t, uint64_t> abs_index) {
    auto N = problem.N;
    auto r = abs_index.first - (N-sum_height(problem, id_worker));
    auto c = abs_index.second - (N-sum_height(problem, id_worker));
    return index(r, c, problem.rel_N);
}

inline uint64_t abs_to_rel_index_t(const problem &problem, int id_worker, std::pair<uint64_t, uint64_t> abs_index) {
    auto N = problem.N;
    auto r = abs_index.first;
    auto c = abs_index.second;
    return index(r, c, problem.rel_N);
}

inline uint64_t rel_matrix_size(const std::vector<uint64_t> &sum_height, int id_worker) {
    auto rel_N = sum_height[id_worker];
    return rel_N;
}

inline void compute_element(problem &problem, const uint64_t &m, const uint64_t &k, int id_worker) {
    double c = 0;
    auto N = problem.N;
    #pragma opm parallel for
    for(uint64_t i=0; i<=k; i++){
        auto index_row = abs_to_rel_index(problem, id_worker, std::pair(m, m+i));
        auto index_col = abs_to_rel_index_t(problem, id_worker, std::pair(N-(m+k)-1, N-(m+k-i)-1));
        c += problem.M1[index_row]*problem.M2[index_col];
    }
    c = std::cbrt(c);
    auto index_element = abs_to_rel_index(problem, id_worker, std::pair(m, m+k));
    auto index_element2 = abs_to_rel_index_t(problem, id_worker, std::pair(N-(m+k)-1, N-m-1));
    problem.M1[index_element] = c;
    problem.M2[index_element2] = c;
}





void wavefront_t(problem &problem, int id_worker) {
    auto index = index_t(problem, id_worker);
    auto leg_size = leg_size_t(problem, id_worker);
    
    for(uint64_t k=1; k<leg_size; k++){
        for(uint64_t m=index; m<index+leg_size-k; m++){
            compute_element(problem, m, k, id_worker);
        }
    }
}

void wavefront_r(problem &problem, int id_worker, uint64_t iter) {
    auto index = index_r(problem, id_worker, iter);
    auto size = size_r(problem, id_worker, iter);

    for(uint64_t c=index.second; c<index.second+size.second; c++){
        for(uint64_t r=index.first+size.first; r>index.first; r--){  
            compute_element(problem, r-1, c-r+1, id_worker);
        }
    }
}

void send_t(const problem &problem, int id_worker, int id_w_target) {
    auto index = index_t(problem, id_worker);
    auto size = leg_size_t(problem, id_worker);

    for(uint64_t r=0; r<size; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(index+r, index));
        MPI_Send(&problem.M1[rel_index], size, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD);
    }
}

void recv_t(problem &problem, int id_worker, int id_w_target) {
    auto index = index_t(problem, id_w_target);
    auto size = leg_size_t(problem, id_w_target);
    auto N = problem.N;

    for(uint64_t r=0; r<size; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(index+r, index));
        MPI_Recv(&problem.M1[rel_index], size, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    for(uint64_t r=0; r<size; r++){
        for(uint64_t c=r; c<size; c++){
            auto rel_index_m1 = abs_to_rel_index(problem, id_worker, std::pair(index+r, index+c));
            auto rel_index_m2 = abs_to_rel_index_t(problem, id_worker, std::pair(N-(index+c)-1, N-(index+r)-1));
            problem.M2[rel_index_m2]=problem.M1[rel_index_m1];
        }   
    }
}


void send_r(const problem &problem, int id_worker, uint64_t iter, int id_w_target) {
    auto index = index_r(problem, id_worker, iter);
    auto size = size_r(problem, id_worker, iter);

    for(uint64_t r=index.first; r<index.first+size.first; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(r, index.second));
        MPI_Send(&problem.M1[rel_index], size.second, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD);
    }
}    

void recv_r(problem &problem, int id_worker, uint64_t iter, int id_w_target) {
    auto index = index_r(problem, id_w_target, iter);
    auto size = size_r(problem, id_w_target, iter);
    auto N = problem.N;

    for(uint64_t r=index.first; r<index.first+size.first; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(r, index.second));
        MPI_Recv(&problem.M1[rel_index], size.second, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    for(uint64_t r=index.first; r<index.first+size.first; r++){
        for(uint64_t c=index.second; c<index.second+size.second; c++){
            auto rel_index_m1 = abs_to_rel_index(problem, id_worker, std::pair(r, c));
            auto rel_index_m2 = abs_to_rel_index_t(problem, id_worker, std::pair(N-(c)-1, N-(r)-1));
            problem.M2[rel_index_m2]=problem.M1[rel_index_m1];
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

    auto sum_height_val = std::vector(n_worker+1, uint64_t(0));
    const auto ROOT_POWER = 1.0;  // da vedere
    for(int id_worker=0; id_worker<n_worker+1; id_worker++){
        // auto sum_height = double(N)*(std::pow(double(n_worker-id_worker)/n_worker, 1.0));  da vedere
        auto sum_height = double(N)*(double(n_worker-id_worker)/n_worker);
        sum_height_val[id_worker] = std::floor(sum_height);
    }

    auto height_val = std::vector(n_worker+1, uint64_t(0));
    for(int id_worker=0; id_worker<n_worker+1; id_worker++){
        auto height = sum_height_val[id_worker]-sum_height_val[id_worker+1];
        height_val[id_worker] = height;
    }
    
    // allocate the matrix
    auto rel_N = rel_matrix_size(sum_height_val, id_worker);
    std::vector<double> M1(rel_N*rel_N, 0);
    std::vector<double> M2(rel_N*rel_N, 0);

    auto p = problem(
        M1, M2, N, rel_N, n_worker,
        sum_height_val, height_val
    );
    auto problem = p;

    // initialize the matrix
    for(uint64_t k=problem.N-sum_height(problem, id_worker); k<problem.N; k++){
        auto index1 = abs_to_rel_index(problem, id_worker, std::pair(k, k));
        auto index2 = abs_to_rel_index_t(problem, id_worker, std::pair(problem.N-k-1, problem.N-k-1));
        problem.M1[index1] = double(k+1)/problem.N;
        problem.M2[index2] = double(k+1)/problem.N;
    }


    START(timer);
    // call the wavefront function

    wavefront_t(problem, id_worker);

    for(int id_w_target = 0; id_w_target<id_worker; id_w_target++){
        send_t(problem, id_worker, id_w_target);
    }

    for(int id_w_target = id_worker+1; id_w_target<n_worker; id_w_target++){
        recv_t(problem, id_worker, id_w_target);
    }

    for(uint64_t iter=0; iter<n_worker-id_worker-1; iter++){
        wavefront_r(problem, id_worker, iter);

        for(int id_w_target = 0; id_w_target<id_worker; id_w_target++){
            send_r(problem, id_worker, iter, id_w_target);
        }

        for(int id_w_target = id_worker+1; id_w_target<n_worker-iter-1; id_w_target++){
            recv_r(problem, id_worker, iter, id_w_target);
        }

    }



    STOP(timer, elapsed);

    if(id_worker==0){
        std::cout << "Elapsed time: " << elapsed << " usec" << std::endl;
        std::cout << "cell:" << M1[N-1] << std::endl;
    }
    

    if(N < 10){
        // print the matrix
        std::cout << "worker " << id_worker << std::endl;
        print_matrix(problem.M1, problem.rel_N);
    }

    MPI_Finalize();

}