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


template <class BV>
int run(sdsl::bit_vector &bv, std::string index_file) {

    typedef typename BV::rank_1_type Rank;
    typedef typename BV::select_1_type Select;
    typedef typename BV::succ_1_type Succ;

    std::cout << "Building " << index_file << " ..." << std::flush;
    BV bitmap(bv);
    Rank rank;
    Succ succ;
    sdsl::util::init_support(rank, &bitmap);
    sdsl::util::init_support(succ, &bitmap);
    std::cout << "[done]" << std::endl;

    std::cout << "Storing " << index_file << " ..." << std::flush;
    std::string rank_file_name = index_file + ".rank";
    std::string succ_file_name = index_file + ".succ";
    sdsl::store_to_file(bitmap, index_file);
    sdsl::store_to_file(rank, rank_file_name);
    sdsl::store_to_file(succ, succ_file_name);
    std::cout << "[done]" << std::endl;

    std::cout << "Size in Bytes: " << (sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ))<< std::endl;
    return 1;

}

template <class BV, class Rank, class Succ>
int run(sdsl::bit_vector &bv, std::string index_file) {

    std::cout << "Building " << index_file << " ..." << std::flush;
    BV bitmap(bv);
    Rank rank;
    Succ succ;
    sdsl::util::init_support(rank, &bitmap);
    sdsl::util::init_support(succ, &bitmap);
    std::cout << "[done]" << std::endl;

    std::cout << "Storing " << index_file << " ..." << std::flush;
    std::string rank_file_name = index_file + ".rank";
    std::string succ_file_name = index_file + ".succ";
    sdsl::store_to_file(bitmap, index_file);
    sdsl::store_to_file(rank, rank_file_name);
    sdsl::store_to_file(succ, succ_file_name);
    std::cout << "[done]" << std::endl;

    std::cout << "Size in Bytes: " << (sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ))<< std::endl;
    return 1;

}

int main(int argc, char** argv)
{
    std::string file_name = argv[1];
    sdsl::bit_vector bv;
    if(!sdsl::load_from_file(bv, file_name)){
        std::cout << "Error: " << file_name << " does not exist." << std::endl;
        exit(0);
    };

    std::cout << "---- HYB ----" << std::endl;
    std::string index_file = file_name + ".hyb";
    run<hyb_type>(bv, index_file);

    index_file = file_name + ".rrr";
    std::cout << "---- RRR ----" << std::endl;
    run<rrr_type>(bv, index_file);

    index_file = file_name + ".oz";
    std::cout << "---- OZ ----" << std::endl;
    run<oz_type>(bv, index_file);

    index_file = file_name + ".sd";
    std::cout << "---- SD ----" << std::endl;
    run<sd_type, sd_rank_type, sd_succ_type>(bv, index_file);

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit";
    run<zombit_plain_type>(bv, index_file);

    std::cout << "---- Zombit Naive ----" << std::endl;
    index_file = file_name + ".zombit.naive";
    run<zombit_plain_type, zombit_plain_rank_type, zombit_plain_succ_naive_type> (bv, index_file);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr";
    run<zombit_rrr_type>(bv, index_file);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb";
    run<zombit_hyb_type>(bv, index_file);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit.naive";
    run<pzombit_plain_type>(bv, index_file);

    std::cout << "---- Zombit v2 ----" << std::endl;
    index_file = file_name + ".zombit.v2";
    run<zombit_v2_plain_type>(bv, index_file);

    std::cout << "---- Zombit v2 Naive ----" << std::endl;
    index_file = file_name + ".zombit.v2.naive";
    run<zombit_v2_plain_type, zombit_v2_plain_rank_type, zombit_v2_plain_succ_naive_type> (bv, index_file);

    std::cout << "---- Zombit v2 RRR----" << std::endl;
    index_file = file_name + ".zombit.v2.rrr";
    run<zombit_v2_rrr_type>(bv, index_file);

    std::cout << "---- Zombit v2 HYB----" << std::endl;
    index_file = file_name + ".zombit.v2.hyb";
    run<zombit_v2_hyb_type>(bv, index_file);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit";
    run<pzombit_plain_type>(bv, index_file);

    std::cout << "---- PZombit Naive ----" << std::endl;
    index_file = file_name + ".pzombit.naive";
    run<pzombit_plain_type, pzombit_plain_rank_type, pzombit_plain_succ_naive_type> (bv, index_file);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr";
    run<pzombit_rrr_type>(bv, index_file);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb";
    run<pzombit_hyb_type>(bv, index_file);

    std::cout << "---- PZombit v2 ----" << std::endl;
    index_file = file_name + ".pzombit.v2";
    run<pzombit_sparse_plain_type>(bv, index_file);

    std::cout << "---- PZombit v2 Naive ----" << std::endl;
    index_file = file_name + ".pzombit.v2.naive";
    run<pzombit_sparse_plain_type, pzombit_sparse_plain_rank_type, pzombit_sparse_plain_succ_naive_type> (bv, index_file);

    std::cout << "---- PZombit v2 RRR----" << std::endl;
    index_file = file_name + ".pzombit.v2.rrr";
    run<pzombit_sparse_rrr_type>(bv, index_file);

    std::cout << "---- PZombit v2 HYB----" << std::endl;
    index_file = file_name + ".pzombit.v2.hyb";
    run<pzombit_sparse_hyb_type>(bv, index_file);

}