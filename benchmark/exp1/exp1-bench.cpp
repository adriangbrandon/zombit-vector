//
// Created by Adrián on 12/2/24.
//

#include <zombit_vector_v3.hpp>
#include <zombit_vector_v4.hpp>
#include <partitioned_zombit_vector.hpp>
#include <partitioned_zombit_vector_sparse.hpp>
#include <sdsl/hyb_vector.hpp>
#include <sdsl/rrr_vector.hpp>
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

std::vector<uint64_t> queries(uint64_t max, uint64_t size){
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, max);
    auto dice = bind(distribution, rng);
    std::vector<uint64_t> res;
    while(res.size() < size){
        res.push_back(dice());
    }
    return res;
}

template <class BV>
void run(std::vector<uint64_t> &qs, std::string index_file){

    typedef typename BV::rank_1_type Rank;
    typedef typename BV::select_1_type Select; //TODO: revisar o select cos zombit (dependen do rank non do bitmap)
    typedef typename BV::succ_1_type Succ;

    BV bitmap;
    Rank rank;
    Succ succ;

    std::string rank_file_name = index_file + ".rank";
    std::string succ_file_name = index_file + ".succ";
    sdsl::load_from_file(bitmap, index_file);
    sdsl::load_from_file(rank, rank_file_name);
    sdsl::load_from_file(succ, succ_file_name);
    rank.set_vector(&bitmap);
    succ.set_vector(&bitmap);

    auto size = sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ);

    uint64_t sum = 0;
    double avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += bitmap[qs[i]];
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += bitmap[qs[i]];
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Access " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_access = avg / TIMES;
    std::cout << "Access avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += rank(qs[i]+1);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += rank(qs[i]+1);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Rank " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_rank = avg / TIMES;
    std::cout << "Rank avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;


    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += succ(qs[i]);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += succ(qs[i]);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Succ " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    std::cout << "Succ avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;
    auto avg_succ = avg / TIMES;

    std::cerr << size << ";" << avg_access << ";" << avg_rank << ";" << avg_succ << std::endl;
}


template<class Bitmap, class Rank, class Succ>
void run_naive(std::vector<uint64_t> &qs, std::string &index_file){

    Bitmap bitmap;
    Rank rank;
    Succ succ;

    std::string rank_file_name = index_file + ".rank";
    sdsl::load_from_file(bitmap, index_file);
    sdsl::load_from_file(rank, rank_file_name);
    rank.set_vector(&bitmap);
    sdsl::util::init_support(succ, &bitmap);

    auto size = sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ);

    uint64_t sum = 0;
    double avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += bitmap[qs[i]];
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += bitmap[qs[i]];
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Access " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_access = avg / TIMES;
    std::cout << "Access avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += rank(qs[i]+1);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += rank(qs[i]+1);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Rank " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_rank = avg / TIMES;
    std::cout << "Rank avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;


    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += succ(qs[i]);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += succ(qs[i]);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Succ " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    std::cout << "Succ avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;
    auto avg_succ = avg / TIMES;

    std::cerr << size << ";" << avg_access << ";" << avg_rank << ";" << avg_succ << std::endl;
}


int main(int argc, char** argv)
{
    std::string file_name = argv[1];
    uint64_t max = atoll(argv[2]);
    uint64_t size = atoll(argv[3]);
    auto q = queries(max, size);

    std::cout << "---- HYB ----" << std::endl;
    std::string index_file = file_name + ".hyb";
    run<hyb_type>(q, index_file);

    index_file = file_name + ".rrr";
    std::cout << "---- RRR ----" << std::endl;
    run<rrr_type>(q, index_file);

    index_file = file_name + ".oz";
    std::cout << "---- OZ ----" << std::endl;
    run<oz_type>(q, index_file);

    index_file = file_name + ".sd";
    std::cout << "---- SD ----" << std::endl;
    run_naive<sd_type, sd_rank_type, sd_succ_type>(q, index_file);

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit";
    run<zombit_plain_type>(q, index_file);

    std::cout << "---- Zombit Naive ----" << std::endl;
    index_file = file_name + ".zombit.naive";
    run_naive<zombit_plain_type, zombit_plain_rank_type, zombit_plain_succ_naive_type> (q, index_file);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr";
    run<zombit_rrr_type>(q, index_file);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb";
    run<zombit_hyb_type>(q, index_file);

    std::cout << "---- Zombit v2 ----" << std::endl;
    index_file = file_name + ".zombit.v2";
    run<zombit_v2_plain_type>(q, index_file);

    std::cout << "---- Zombit v2 Naive ----" << std::endl;
    index_file = file_name + ".zombit.v2.naive";
    run_naive<zombit_v2_plain_type, zombit_v2_plain_rank_type, zombit_v2_plain_succ_naive_type> (q, index_file);

    std::cout << "---- Zombit v2 RRR----" << std::endl;
    index_file = file_name + ".zombit.v2.rrr";
    run<zombit_v2_rrr_type>(q, index_file);

    std::cout << "---- Zombit v2 HYB----" << std::endl;
    index_file = file_name + ".zombit.v2.hyb";
    run<zombit_v2_hyb_type>(q, index_file);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit";
    run<pzombit_plain_type>(q, index_file);

    std::cout << "---- PZombit Naive ----" << std::endl;
    index_file = file_name + ".pzombit.naive";
    run_naive<pzombit_plain_type, pzombit_plain_rank_type, pzombit_plain_succ_naive_type> (q, index_file);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr";
    run<pzombit_rrr_type>(q, index_file);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb";
    run<pzombit_hyb_type>(q, index_file);

    std::cout << "---- PZombit v2 ----" << std::endl;
    index_file = file_name + ".pzombit.v2";
    run<pzombit_sparse_plain_type>(q, index_file);

    std::cout << "---- PZombit v2 Naive ----" << std::endl;
    index_file = file_name + ".pzombit.v2.naive";
    run_naive<pzombit_sparse_plain_type, pzombit_sparse_plain_rank_type, pzombit_sparse_plain_succ_naive_type> (q, index_file);

    std::cout << "---- PZombit v2 RRR----" << std::endl;
    index_file = file_name + ".pzombit.v2.rrr";
    run<pzombit_sparse_rrr_type>(q, index_file);

    std::cout << "---- PZombit v2 zHYB----" << std::endl;
    index_file = file_name + ".pzombit.v2.hyb";
    run<pzombit_sparse_hyb_type>(q, index_file);



}

