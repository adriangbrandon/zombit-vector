
/***
BSD 2-Clause License

Copyright (c) 2018, Adrián
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/


//
// Created by Adrián on 27/7/22.
//


#include <string>
#include <succ-vec/rrr_vector.hpp>
#include <succ-vec/hyb_vector.hpp>
#include <sdsl/construct.hpp>
#include <iostream>
#include <zombit_vector_v3.hpp>
#include <rec_partitioned_zombit_vector.hpp>
#include <partitioned_zombit_vector.hpp>
#include "sdsl/wm_int.hpp"

typedef runs_vectors::zombit_vector_v3<> zombit_plain_type;
typedef runs_vectors::zombit_vector_v3<sdsl_v2::rrr_vector<127>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl_v2::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl_v2::rrr_vector<127>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl_v2::hyb_vector<>> pzombit_hyb_type;
typedef sdsl_v2::rrr_vector<127> rrr_type;
typedef sdsl_v2::hyb_vector<> hyb_type;


void build_bwt(const sdsl::int_vector<> &text, sdsl::int_vector<> &bwt){
    sdsl::int_vector<> sa;
    sdsl::qsufsort::construct_sa(sa, text);

    for (uint64_t i = 0; i < sa.size(); ++i) {
        if (sa[i] == 0){
            bwt[i] = text[text.size()-1];
        }else{
            bwt[i] = text[sa[i]-1];
        }
    }
}

void read_file(const std::string &file_name, uint8_t num_bytes, sdsl::int_vector<> &text){
    sdsl::load_vector_from_file(text, file_name, num_bytes);
    uint64_t id = 1;
    std::unordered_map<uint64_t, uint64_t > map;
    for(uint64_t i = 0; i < text.size(); ++i){
        auto it = map.find(text[i]);
        if(it != map.end()){
            text[i] = it->second;
        }else{
            map.insert({text[i], id});
            text[i] = id;
            ++id;
        }
    }
    sdsl::append_zero_symbol(text);
}



template <class BV>
int run(std::string file_name, std::string index_file) {


    sdsl::wt_huff<BV> wm_bv;
    std::cout << "Reading WM ... " << std::flush;
    if(!sdsl::load_from_file(wm_bv, index_file)){
        std::cout << "[fail]" << std::endl;
        sdsl::int_vector<> text, bwt;
        std::cout << "Building WM ... " << std::flush;
        read_file(file_name, 1, text);
        bwt.resize(text.size());
        build_bwt(text, bwt);
        sdsl::construct_im(wm_bv, bwt);
        sdsl::store_to_file(wm_bv, index_file);
        std::cout << "[done]" << std::endl;
    }else{
        std::cout << "[done]" << std::endl;
    }
    std::cout << "sigma: " << wm_bv.sigma << std::endl;
    std::cout << "Size in Bytes : " << sdsl::size_in_bytes(wm_bv) << std::endl;
    std::ofstream out(index_file + ".html");
    sdsl::write_structure<sdsl::HTML_FORMAT>(wm_bv, out);
    return 1;

}

int main(int argc, char** argv)
{
    std::string file_name = argv[1];

    std::cout << "---- HYB ----" << std::endl;
    std::string index_file = file_name + ".hyb.wt";
    run<hyb_type>(file_name, index_file);

    index_file = file_name + ".rrr.wt";
    std::cout << "---- RRR ----" << std::endl;
    run<rrr_type>(file_name, index_file);

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit.wt";
    run<zombit_plain_type>(file_name, index_file);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr.wt";
    run<zombit_rrr_type>(file_name, index_file);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb.wt";
    run<zombit_hyb_type>(file_name, index_file);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit.wt";
    run<pzombit_plain_type>(file_name, index_file);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr.wt";
    run<pzombit_rrr_type>(file_name, index_file);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb.wt";
    run<pzombit_hyb_type>(file_name, index_file);

}