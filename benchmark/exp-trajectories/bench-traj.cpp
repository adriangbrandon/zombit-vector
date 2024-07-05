//
// Created by Adrián on 27/2/24.
//

#include <file.hpp>
#include <sstream>
#include <sdsl/int_vector.hpp>

#include <sdsl/suffix_arrays.hpp>
#include <zombit_vector_v3.hpp>
#include <zombit_vector_v4.hpp>
#include <partitioned_zombit_vector.hpp>
#include <partitioned_zombit_vector_sparse.hpp>
#include <succ_support_sd.hpp>
#include <oz_vector.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type;
typedef runs_vectors::succ_support_zombit_v3_naive zombit_plain_succ_naive_type;
typedef typename runs_vectors::zombit_vector_v3<sdsl::bit_vector>::rank_1_type zombit_plain_rank_type;
typedef runs_vectors::zombit_vector_v4<sdsl::bit_vector> zombit_v2_plain_type;
typedef runs_vectors::succ_support_zombit_v4_naive zombit_v2_plain_succ_naive_type;
typedef typename runs_vectors::zombit_vector_v4<sdsl::bit_vector>::rank_1_type zombit_v2_plain_rank_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<15>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::zombit_vector_v4<sdsl::rrr_vector<15>> zombit_v2_rrr_type;
typedef runs_vectors::zombit_vector_v4<sdsl::hyb_vector<>> zombit_v2_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_naive pzombit_plain_succ_naive_type;
typedef typename runs_vectors::partitioned_zombit_vector<>::rank_1_type pzombit_plain_rank_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<15>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;

typedef runs_vectors::partitioned_zombit_vector_sparse<> pzombit_sparse_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_sparse_naive pzombit_sparse_plain_succ_naive_type;
typedef typename runs_vectors::partitioned_zombit_vector_sparse<>::rank_1_type pzombit_sparse_plain_rank_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<sdsl::rrr_vector<15>> pzombit_sparse_rrr_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<sdsl::hyb_vector<>> pzombit_sparse_hyb_type;

typedef sdsl::rrr_vector<15> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;
typedef sdsl::sd_vector<> sd_type;
typedef sdsl::sd_vector<>::rank_1_type sd_rank_type;
typedef sdsl::succ_support_sd<> sd_succ_type;
typedef runs_vectors::oz_vector<> oz_type;

#define TIMES 5

void queries(uint64_t max, uint64_t len, uint64_t size, std::vector<std::pair<int, int>> &res){
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, max);
    auto dice = bind(distribution, rng);
    while(res.size() < size){
        auto b = dice();
        res.emplace_back(b, b+len);
    }
}


void print(std::vector<uint64_t> &vec){

    for(const auto &v : vec){
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template<class BV, class Succ, class Rank>
uint64_t load_bitmap(BV &bitmap, Rank &rank, Succ &succ, const std::string &file_name){

    std::string rank_file_name = file_name + ".rank";
    std::string succ_file_name = file_name + ".succ";

    sdsl::load_from_file(bitmap, file_name);
    sdsl::load_from_file(rank, rank_file_name);
    sdsl::load_from_file(succ, succ_file_name);
    rank.set_vector(&bitmap);
    succ.set_vector(&bitmap);
    return bitmap.size();
}

template<class BV, class Rank>
uint64_t exists(BV &bitmap, Rank &rank, uint64_t i) {
    if(bitmap[i]) {
        return rank(i+1)-1;
    }
    return 0;
}

template<class BV, class Succ, class Rank>
std::vector<uint64_t> range(BV &bitmap, Rank &rank, Succ &succ, uint64_t i, uint64_t j) {
    std::vector<uint64_t> res;
    j = std::min(j, bitmap.size());
    auto b = rank(i+1)-1;
    auto t = succ(i);
    while(t < j) {
        res.push_back(b);
        ++b;
        t = succ(t+1);
    }
    return res;
}

template <class BV, class Succ, class Rank>
void run(const std::string &path, std::vector<std::pair<int, int>> &qs){

    BV bitmap;
    Rank rank;
    Succ succ;

    std::cout << "Reading BV... " << std::flush;
    auto max = load_bitmap(bitmap, rank, succ, path);
    if(!max){
        std::cout << " [fail]." << std::endl;
        exit(0);
    }
    std::cout << " [done]." << std::endl;

    if(qs.empty()) {
        std::cout << "Max: " << max << std::endl;
        std::cout << "Build queries... " << std::flush;
        queries(max, 100, 1000, qs);
        std::cout << " [done]." << std::endl;
    }

    auto bytes = sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ);

    std::cout << "--- Exists queries ---" << std::endl;

    double avg = 0;
    uint64_t sum;
    for(uint64_t t = 0; t < TIMES; ++t) {
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t k = 0; k < qs.size(); ++k) {
            sum += exists(bitmap, rank, qs[k].first);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_q = time / (double) qs.size();
        avg += per_q;
        std::cout << "Eixsts " << time << " ns (per query: " << per_q << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_exists = avg / TIMES;
    std::cout << "Exists avg per query: " << avg_exists << " ns." << std::endl;
    std::cout << std::endl;

    std::vector<uint64_t> res;
    for(uint64_t t = 0; t < TIMES; ++t) {
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t k = 0; k < qs.size(); ++k) {
            res = range(bitmap, rank, succ, qs[k].first, qs[k].second);
            sum += res.size();
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_q = time / (double) qs.size();
        avg += per_q;
        std::cout << "Range " << time << " ns (per query: " << per_q << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_range = avg / TIMES;
    std::cout << "Range avg per query: " << avg_range << " ns." << std::endl;
    std::cout << std::endl;


    std::cerr << bytes << ";" << avg_exists << ";" << avg_range << std::endl;
}





int main(int argc, char** argv){
    std::string file_name = argv[1];

    std::vector<std::pair<int, int>> qs;

    std::string index_file = file_name + ".hyb";
    run<hyb_type, hyb_type::succ_1_type, hyb_type::rank_1_type>(index_file, qs);

    index_file = file_name + ".rrr";
    std::cout << "---- RRR ----" << std::endl;
    run<rrr_type, rrr_type::succ_1_type, rrr_type::rank_1_type>(index_file, qs);

    index_file = file_name + ".oz";
    std::cout << "---- OZ ----" << std::endl;
    run<oz_type, oz_type::succ_1_type, oz_type::rank_1_type>(index_file, qs);

    index_file = file_name + ".sd";
    std::cout << "---- SD ----" << std::endl;
    run<sd_type, sd_succ_type, sd_rank_type>(index_file, qs);

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit";
    run<zombit_plain_type, zombit_plain_type::succ_1_type, zombit_plain_type::rank_1_type>(index_file, qs);

    std::cout << "---- Zombit Naive ----" << std::endl;
    index_file = file_name + ".zombit.naive";
    run<zombit_plain_type, zombit_plain_succ_naive_type, zombit_plain_rank_type> (index_file, qs);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr";
    run<zombit_rrr_type, zombit_rrr_type::succ_1_type, zombit_rrr_type::rank_1_type>(index_file, qs);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb";
    run<zombit_hyb_type, zombit_hyb_type::succ_1_type, zombit_hyb_type::rank_1_type>(index_file, qs);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit.naive";
    run<pzombit_plain_type, pzombit_plain_type::succ_1_type, pzombit_plain_type::rank_1_type>(index_file, qs);

    std::cout << "---- Zombit v2 ----" << std::endl;
    index_file = file_name + ".zombit.v2";
    run<zombit_v2_plain_type, zombit_v2_plain_type::succ_1_type, zombit_v2_plain_type::rank_1_type>(index_file, qs);

    std::cout << "---- Zombit v2 Naive ----" << std::endl;
    index_file = file_name + ".zombit.v2.naive";
    run<zombit_v2_plain_type, zombit_v2_plain_succ_naive_type, zombit_v2_plain_rank_type> (index_file, qs);

    std::cout << "---- Zombit v2 RRR----" << std::endl;
    index_file = file_name + ".zombit.v2.rrr";
    run<zombit_v2_rrr_type, zombit_v2_rrr_type::succ_1_type, zombit_v2_rrr_type::rank_1_type> (index_file, qs);

    std::cout << "---- Zombit v2 HYB----" << std::endl;
    index_file = file_name + ".zombit.v2.hyb";
    run<zombit_v2_hyb_type, zombit_v2_hyb_type::succ_1_type, zombit_v2_hyb_type::rank_1_type> (index_file, qs);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit";
    run<pzombit_plain_type, pzombit_plain_type::succ_1_type, pzombit_plain_type::rank_1_type> (index_file, qs);

    std::cout << "---- PZombit Naive ----" << std::endl;
    index_file = file_name + ".pzombit.naive";
    run<pzombit_plain_type, pzombit_plain_succ_naive_type, pzombit_plain_rank_type> (index_file, qs);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr";
    run<pzombit_rrr_type, pzombit_rrr_type::succ_1_type, pzombit_rrr_type::rank_1_type> (index_file, qs);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb";
    run<pzombit_hyb_type, pzombit_hyb_type::succ_1_type, pzombit_hyb_type::rank_1_type> (index_file, qs);

    std::cout << "---- PZombit v2 ----" << std::endl;
    index_file = file_name + ".pzombit.v2";
    run<pzombit_sparse_plain_type, pzombit_sparse_plain_type::succ_1_type, pzombit_sparse_plain_type::rank_1_type> (index_file, qs);

    std::cout << "---- PZombit v2 Naive ----" << std::endl;
    index_file = file_name + ".pzombit.v2.naive";
    run<pzombit_sparse_plain_type, pzombit_sparse_plain_succ_naive_type, pzombit_sparse_plain_rank_type> (index_file, qs);

    std::cout << "---- PZombit v2 RRR----" << std::endl;
    index_file = file_name + ".pzombit.v2.rrr";
    run<pzombit_sparse_rrr_type, pzombit_sparse_rrr_type::succ_1_type, pzombit_sparse_rrr_type::rank_1_type> (index_file, qs);

    std::cout << "---- PZombit v2 HYB----" << std::endl;
    index_file = file_name + ".pzombit.v2.hyb";
    run<pzombit_sparse_hyb_type, pzombit_sparse_hyb_type::succ_1_type, pzombit_sparse_hyb_type::rank_1_type> (index_file, qs);


}
