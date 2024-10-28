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
#include <utils_mpi_w0.hpp>


void send_t(const problem &problem, int id_worker, int id_w_target, std::vector<MPI_Request> &requests) {
    auto index = index_t(problem, id_worker);
    auto size = leg_size_t(problem, id_worker);

    for(uint64_t r=0; r<size-1; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(index+r, index+r+1));
        requests.push_back(MPI_Request());
        MPI_Isend(&problem.M[rel_index], size-r-1, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD, &requests.back());
    }
}

void recv_t(problem &problem, int id_worker, int id_w_target, std::vector<MPI_Request> &requests) {
    auto index = index_t(problem, id_w_target);
    auto size = leg_size_t(problem, id_w_target);

    for(uint64_t r=0; r<size-1; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(index+r, index+r+1));
        requests.push_back(MPI_Request());
        MPI_Irecv(&problem.M[rel_index], size-r-1, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD, &requests.back());
    }
}

void send_r(const problem &problem, int id_worker, uint64_t iter, int id_w_target, std::vector<MPI_Request> &requests) {
    auto index = index_r(problem, id_worker, iter);
    auto size = size_r(problem, id_worker, iter);

    for(uint64_t r=index.first; r<index.first+size.first; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(r, index.second));
        requests.push_back(MPI_Request());
        MPI_Isend(&problem.M[rel_index], size.second, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD, &requests.back());
    }
}    

void recv_r(problem &problem, int id_worker, uint64_t iter, int id_w_target, std::vector<MPI_Request> &requests) {
    auto index = index_r(problem, id_w_target, iter);
    auto size = size_r(problem, id_w_target, iter);

    for(uint64_t r=index.first; r<index.first+size.first; r++){
        auto rel_index = abs_to_rel_index(problem, id_worker, std::pair(r, index.second));
        requests.push_back(MPI_Request());
        MPI_Irecv(&problem.M[rel_index], size.second, MPI_DOUBLE, id_w_target, 0, MPI_COMM_WORLD, &requests.back());
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
    std::vector<double> M(rel_N*rel_N, 0);

    auto p = problem(
        M, N, rel_N, n_worker,
        sum_height_val, height_val
    );
    auto problem = p;

    // initialize the matrix
    for(uint64_t k=problem.N-sum_height(problem, id_worker); k<problem.N; k++){
        auto index = abs_to_rel_index(problem, id_worker, std::pair(k, k));
        problem.M[index] = double(k+1)/problem.N;
    }


    START(timer);
    // call the wavefront function

    wavefront_t(problem, id_worker);

    // vettore requests di send e recv
    auto send_size_t = (height(problem, id_worker)-1)*id_worker;
    auto recv_size_t = sum_height(problem, id_worker+1)-1*(n_worker-id_worker-1);
    std::vector<MPI_Request> requests_t;
    requests_t.reserve(send_size_t+recv_size_t);

    for(int id_w_target = 0; id_w_target<id_worker; id_w_target++){
        send_t(problem, id_worker, id_w_target, requests_t);
    }

    for(int id_w_target = id_worker+1; id_w_target<n_worker; id_w_target++){
        recv_t(problem, id_worker, id_w_target, requests_t);
    }

    // wait all
    MPI_Waitall(requests_t.size(), requests_t.data(), MPI_STATUSES_IGNORE);

    for(int id_w_target = id_worker+1; id_w_target<n_worker; id_w_target++){
        flip_t(problem, id_worker, id_w_target);
    }
    

    for(uint64_t iter=0; iter<n_worker-id_worker-1; iter++){

        // vettore requests di send e recv
        auto send_size_r = height(problem, id_worker)*id_worker;
        auto recv_size_r = sum_height(problem, id_worker+1)-sum_height(problem, n_worker-iter-1);
        std::vector<MPI_Request> requests_r;
        requests_r.reserve(send_size_r+recv_size_r);


        wavefront_r(problem, id_worker, iter);

        for(int id_w_target = 0; id_w_target<id_worker; id_w_target++){
            send_r(problem, id_worker, iter, id_w_target, requests_r);
        }

        for(int id_w_target = id_worker+1; id_w_target<n_worker-iter-1; id_w_target++){
            recv_r(problem, id_worker, iter, id_w_target, requests_r);
        }

        // wait all
        MPI_Waitall(requests_r.size(), requests_r.data(), MPI_STATUSES_IGNORE);

        for(int id_w_target = id_worker+1; id_w_target<n_worker-iter-1; id_w_target++){
            flip_r(problem, id_worker, iter, id_w_target);
        }


    }

    STOP(timer, elapsed);

    if(id_worker==0){
        std::cout << "Elapsed time: " << elapsed << " usec" << std::endl;
        std::cout << "cell:" << M[N-1] << std::endl;
    }
    

    if(N < 10){
        // print the matrix
        std::cout << "worker " << id_worker << std::endl;
        print_matrix(problem.M, problem.rel_N);
    }

    MPI_Finalize();

}