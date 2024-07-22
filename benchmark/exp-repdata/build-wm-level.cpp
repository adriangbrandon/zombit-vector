//
// Created by Adrián on 21/2/24.
//

#include <string>
#include <sdsl/suffix_arrays.hpp>
#include <iostream>
#include <zombit_vector_v3.hpp>
#include <partitioned_zombit_vector.hpp>

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


template <class BV>
int run(const std::string &file_name, const std::string &index_name, uint8_t num_bytes) {

    typedef typename BV::rank_1_type Rank;

    sdsl::wm_int<sdsl::bit_vector> wm_bv;
    std::cout << "Reading WM ... " << std::flush;
    if(!sdsl::load_from_file(wm_bv, index_name)){
        std::cout << "[fail]" << std::endl;
        std::cout << "Building WM... " << std::flush;
        sdsl::construct(wm_bv, file_name, num_bytes);
        //sdsl::construct_im(wm_bv, bwt.data(), 1);
        sdsl::store_to_file(wm_bv, index_name);
    }else{
        std::cout << "[done]" << std::endl;
    }
    std::cout << "sigma: " << wm_bv.sigma << std::endl;
    std::cout << "Size in Bytes bitmaps: " << sdsl::size_in_bytes(wm_bv.tree) << std::endl;

    BV bv_wm(wm_bv.tree);
    Rank rank_wm;
    sdsl::util::init_support(rank_wm, &bv_wm);
    //Current bitmaps and ranks
    uint64_t size_wm = sdsl::size_in_bytes(wm_bv.tree) + sdsl::size_in_bytes(rank_wm);

    auto max_level = wm_bv.max_level;
    uint64_t size_wm_by_level = max_level * 8; //
    for(uint64_t l = 0; l < max_level; ++l){
        std::cout << "Building bitvector at level=" << l << " ..." << std::flush;
        sdsl::bit_vector bv(wm_bv.size());
        for(uint64_t p = l*bv.size(); p < (l+1)*bv.size(); ++p){
            bv[p - l*bv.size()] = wm_bv.tree[p];
        }
        std::cout << "[done]" << std::endl;
        std::cout << "Compressing bitvector at level=" << l << " ..." << std::flush;
        BV new_bv(std::move(bv));
        Rank rank;
        sdsl::util::init_support(rank, &new_bv);
        std::cout << "[done]" << std::endl;

        auto curr_level = sdsl::size_in_bytes(new_bv) + sdsl::size_in_bytes(rank);
        std::cout << "Size in Bytes at level=" << l << ": " << curr_level << std::endl;
        size_wm_by_level += curr_level;
    }

    std::cout << "======================================================" << std::endl;
    std::cout << " Size wm: " << size_wm << " bytes." << std::endl;
    std::cout << " Size wm by level: " << size_wm_by_level << " bytes." << std::endl;
    std::cout << "======================================================" << std::endl;

}



int main(int argc, char** argv){
    std::string file_name = argv[1];
    std::string index_name = argv[2];
    std::string type = argv[3];
    uint num_bytes = std::atoi(argv[4]);
    if(type == "zombit-plain"){
        run<zombit_plain_type>(file_name, index_name, num_bytes);
    }else if (type == "zombit-rrr"){
        run<zombit_rrr_type>(file_name, index_name, num_bytes);
    }else if (type == "zombit-hyb"){
        run<zombit_hyb_type>(file_name, index_name, num_bytes);
    }else if (type == "pzombit-plain"){
        run<pzombit_plain_type>(file_name, index_name, num_bytes);
    }else if (type == "pzombit-rrr"){
        run<pzombit_rrr_type>(file_name, index_name, num_bytes);
    }else if (type == "pzombit-hyb"){
        run<pzombit_hyb_type>(file_name, index_name, num_bytes);
    }else if (type == "hyb"){
        run<hyb_type>(file_name, index_name, num_bytes);
    }else if (type == "rrr"){
        run<rrr_type>(file_name, index_name, num_bytes);
    }
    return 0;
}