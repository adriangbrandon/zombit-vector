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
// Created by Adrián on 8/6/21.
//


#include <string>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/bit_vectors.hpp>
#include <iostream>
#include <zombit_vector_v3.hpp>

typedef runs_vectors::zombit_vector_v3 zombit_type;
typedef runs_vectors::rank_support_zombit_v3<1> rank_zombit_type;
typedef sdsl::rrr_vector<127> rrr_type;
typedef sdsl::rrr_vector<127>::rank_1_type rank_rrr_type;
typedef sdsl::hyb_vector<> hyb_type;
typedef sdsl::hyb_vector<>::rank_1_type rank_hyb_type;


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



template <class BV, class Rank>
int run(std::string file_name, std::string index_file) {


    sdsl::wm_int<sdsl::bit_vector> wm_bv;
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
    std::cout << "Size in Bytes WM: " << sdsl::size_in_bytes(wm_bv) << std::endl;
    auto max_level = wm_bv.max_level;
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
        std::cout << "Size in Bytes at level=" << l << ": "
        << sdsl::size_in_bytes(new_bv) + sdsl::size_in_bytes(rank) << std::endl;
    }

}

int main(int argc, char** argv)
{
    std::string file_name = argv[1];
    std::string type = argv[2];
    std::string index_file = file_name + ".wm";
    if(type == "hyb"){
        std::cout << "---- HYB ----" << std::endl;
        run<hyb_type, rank_hyb_type>(file_name, index_file);
    }else if (type == "rrr"){
        std::cout << "---- RRR ----" << std::endl;
        run<rrr_type, rank_rrr_type>(file_name, index_file);
    }else if (type == "zombit"){
        std::cout << "---- Zombit ----" << std::endl;
        run<zombit_type, rank_zombit_type>(file_name, index_file);
    }
}