#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include <utils.hpp>

struct problem {
    std::vector<double> &M;
    uint64_t N;
    uint64_t rel_N;
    int n_worker;
    std::vector<uint64_t> &sum_height;
    std::vector<uint64_t> &height;

    problem(
        std::vector<double> &M,
        uint64_t N,
        uint64_t rel_N,
        int n_worker,
        std::vector<uint64_t> &sum_height,
        std::vector<uint64_t> &height
    ) : M(M), N(N), rel_N(rel_N), n_worker(n_worker), sum_height(sum_height), height(height) {}
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

inline uint64_t rel_matrix_size(const std::vector<uint64_t> &sum_height, int id_worker) {
    auto rel_N = sum_height[id_worker];
    return rel_N;
}

inline void compute_element(problem &problem, const uint64_t &m, const uint64_t &k, int id_worker) {
    double c = 0;
    
    for(uint64_t i=0; i<=k; i++){
        auto index_row = abs_to_rel_index(problem, id_worker, std::pair(m, m+i));
        auto index_col = abs_to_rel_index(problem, id_worker, std::pair(m+k, m+k-i));
        c += problem.M[index_row]*problem.M[index_col];
    }
    c = std::cbrt(c);
    auto index_element = abs_to_rel_index(problem, id_worker, std::pair(m, m+k));
    auto index_element_t = abs_to_rel_index(problem, id_worker, std::pair(m+k, m));
    problem.M[index_element] = c;
    problem.M[index_element_t] = c;
}


void wavefront_t(problem &problem, int id_worker) {
    auto index = index_t(problem, id_worker);
    auto leg_size = leg_size_t(problem, id_worker);
    
    for(uint64_t k=1; k<leg_size; k++){
        #pragma omp parallel for 
        for(uint64_t m=index; m<index+leg_size-k; m++){
            compute_element(problem, m, k, id_worker);
        }
    }
}

void wavefront_r(problem &problem, int id_worker, uint64_t iter) {
    auto index = index_r(problem, id_worker, iter);
    auto size = size_r(problem, id_worker, iter);

// #ifndef USE_OMP
    // for(uint64_t c=index.second; c<index.second+size.second; c++){
    //     for(uint64_t r=index.first+size.first; r>index.first; r--){  
    //         compute_element(problem, r-1, c-r+1, id_worker);
    //     }
    // }
// #else
    for(uint64_t k=0; k<size.second+size.first-1;k++){
        #pragma omp parallel for
        for(int64_t m=std::max(int64_t(0),int64_t(size.first-k-1)); m<std::min(int64_t(size.first),int64_t(size.second+size.first-1-k)); m++){
            auto r = index.first+m;
            auto c = index.second+k-size.first+m+1;
            compute_element(problem, r, c-r, id_worker);
        }
    }
// #endif
}

void flip_t(problem &problem, int id_worker, int id_w_target) {
    auto index = index_t(problem, id_w_target);
    auto size = leg_size_t(problem, id_w_target);

    for(uint64_t r=0; r<size-1; r++){
        for (uint64_t c = r+1; c < size; c++){
            auto index_n = abs_to_rel_index(problem, id_worker, std::pair(index+r, index+c));
            auto index_t = abs_to_rel_index(problem, id_worker, std::pair(index+c, index+r));
            problem.M[index_t]=problem.M[index_n];
        }
    }
}

void flip_r(problem &problem, int id_worker, uint64_t iter, int id_w_target) {
    auto index = index_r(problem, id_w_target, iter);
    auto size = size_r(problem, id_w_target, iter);

    for(uint64_t r=index.first; r<index.first+size.first; r++){
        for (uint64_t c = index.second; c < index.second+size.second; c++){
            auto index_n = abs_to_rel_index(problem, id_worker, std::pair(r, c));
            auto index_t = abs_to_rel_index(problem, id_worker, std::pair(c, r));
            problem.M[index_t]=problem.M[index_n];
        }
    }
}