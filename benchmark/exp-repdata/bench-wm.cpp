//
// Created by Adrián on 21/2/24.
//

#include <string>
#include <sdsl/suffix_arrays.hpp>
#include <iostream>
#include <oz_vector.hpp>
#include <zombit_vector_v3.hpp>
#include <partitioned_zombit_vector.hpp>
#include <file.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type;
typedef runs_vectors::succ_support_zombit_v3_naive zombit_plain_succ_naive_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<127>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_naive pzombit_plain_succ_naive_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<127>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;
typedef sdsl::rrr_vector<127> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;
typedef runs_vectors::oz_vector<> oz_type;

#define TIMES 5

template <class BV>
void run(const std::string &file_random, const std::string &index_name, uint8_t num_bytes){
    sdsl::wm_int<BV> wm_bv;
    std::cout << "Reading WM from " << index_name << std::flush;
    sdsl::load_from_file(wm_bv, index_name);
    std::cout << " [done]." << std::endl;
    std::cout << "sigma: " << wm_bv.sigma << std::endl;
    std::cout << "size: " << wm_bv.size() << std::endl;
    std::cout << "max_level: " << wm_bv.max_level << std::endl;
    auto size = sdsl::size_in_bytes(wm_bv);
    std::cout << "Size in Bytes : " << size << std::endl;

    vector<uint64_t> random; //TODO read from file
    util::file::read_from_file(file_random, random);

    vector<uint64_t> q(random.size()), c(random.size());
    for(uint64_t i = 0; i < random.size(); ++i){
        q[i] = random[i] % wm_bv.size();
        c[i] = random[i] % wm_bv.sigma;
    }

    uint64_t sum = 0;
    double avg = 0;
    for(uint64_t t = 0; t < TIMES; ++t) {
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < random.size(); ++i){
            sum += wm_bv.rank(q[i], c[i]);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_q = time / (double) q.size();
        avg += per_q;
        std::cout << "Rank " << time << " ns (per query: " << per_q << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_rank = avg / TIMES;
    std::cout << "Rank avg per query: " << avg_rank << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0;
    avg = 0;
    for(uint64_t t = 0; t < TIMES; ++t) {
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < random.size(); ++i){
            sum += wm_bv[q[i]];
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_q = time / (double) q.size();
        avg += per_q;
        std::cout << "Access " << time << " ns (per query: " << per_q << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_access = avg / TIMES;
    std::cout << "Access avg per query: " << avg_access << " ns." << std::endl;
    std::cout << std::endl;

}

int main(int argc, char** argv){
    std::string file_random = argv[4];
    std::string index_name = argv[1];
    std::string type = argv[2];
    uint num_bytes = std::atoi(argv[3]);
    if(type == "zombit-plain"){
        run<zombit_plain_type>(file_random, index_name, num_bytes);
    }else if (type == "zombit-rrr"){
        run<zombit_rrr_type>(file_random, index_name, num_bytes);
    }else if (type == "zombit-hyb"){
        run<zombit_hyb_type>(file_random, index_name, num_bytes);
    }else if (type == "pzombit-plain"){
        run<pzombit_plain_type>(file_random, index_name, num_bytes);
    }else if (type == "pzombit-rrr"){
        run<pzombit_rrr_type>(file_random, index_name, num_bytes);
    }else if (type == "pzombit-hyb"){
        run<pzombit_hyb_type>(file_random, index_name, num_bytes);
    }else if (type == "hyb"){
        run<hyb_type>(file_random, index_name, num_bytes);
    }else if (type == "rrr"){
        run<rrr_type>(file_random, index_name, num_bytes);
    }else if (type == "oz") {
        run<oz_type>(file_random, index_name, num_bytes);
    }
}