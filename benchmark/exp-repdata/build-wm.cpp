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
void run(const std::string &file_name, const std::string &index_name){

    sdsl::wm_int<BV> wm_bv;
    std::cout << "Reading WM... " << std::flush;
    if(!sdsl::load_from_file(wm_bv, index_name)){
        std::cout << " [fail]." << std::endl;
        /*std::cout << "Loading bwt... " << std::flush;
        sdsl::int_vector<8> bwt;
        sdsl::load_from_file(bwt, file_name);
        std::cout << " [done]." << std::endl;
        int sigma = 0;
        for(auto &a : bwt){
            if(a > sigma) sigma = a;
        }
        std::cout << "sigma de verdade: " << sigma << std::endl;
        std::cout << bwt.size() << std::endl;
        std::cout << (uint64_t) bwt.width() << std::endl;*/
        std::cout << "Building WM... " << std::flush;
        sdsl::construct(wm_bv, file_name, 1);
        //sdsl::construct_im(wm_bv, bwt.data(), 1);
        sdsl::store_to_file(wm_bv, index_name);
    }
    std::cout << " [done]." << std::endl;


    std::cout << "sigma: " << wm_bv.sigma << std::endl;
    std::cout << "Size in Bytes : " << sdsl::size_in_bytes(wm_bv) << std::endl;
    std::ofstream out(index_name + ".html");
    sdsl::write_structure<sdsl::HTML_FORMAT>(wm_bv, out);
}

int main(int argc, char** argv){
    std::string file_name = argv[1];
    std::string index_name = argv[2];
    std::string type = argv[3];
    if(type == "zombit-plain"){
        run<zombit_plain_type>(file_name, index_name);
    }else if (type == "zombit-rrr"){
        run<zombit_rrr_type>(file_name, index_name);
    }else if (type == "zombit-hyb"){
        run<zombit_hyb_type>(file_name, index_name);
    }else if (type == "pzombit-plain"){
        run<pzombit_plain_type>(file_name, index_name);
    }else if (type == "pzombit-rrr"){
        run<pzombit_rrr_type>(file_name, index_name);
    }else if (type == "pzombit-hyb"){
        run<pzombit_hyb_type>(file_name, index_name);
    }else if (type == "hyb"){
        run<hyb_type>(file_name, index_name);
    }else if (type == "rrr"){
        run<rrr_type>(file_name, index_name);
    }
}