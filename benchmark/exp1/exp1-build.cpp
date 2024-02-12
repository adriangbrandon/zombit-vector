//
// Created by Adrián on 12/2/24.
//

#include <zombit_vector_v3.hpp>
#include <partitioned_zombit_vector.hpp>
#include <sdsl/hyb_vector.hpp>
#include <sdsl/rrr_vector.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<127>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<127>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;
typedef sdsl::rrr_vector<127> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;


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

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit";
    run<zombit_plain_type>(bv, index_file);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr";
    run<zombit_rrr_type>(bv, index_file);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb";
    run<zombit_hyb_type>(bv, index_file);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit";
    run<pzombit_plain_type>(bv, index_file);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr";
    run<pzombit_rrr_type>(bv, index_file);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb";
    run<pzombit_hyb_type>(bv, index_file);

}