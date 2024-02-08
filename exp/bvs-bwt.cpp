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
#include <partitioned_zombit_vector.hpp>
#include "sdsl/wm_int.hpp"
#include <file.hpp>

typedef runs_vectors::zombit_vector_v3<> zombit_plain_type;
typedef runs_vectors::zombit_vector_v3<sdsl_v2::rrr_vector<127>, sdsl_v2::rrr_vector<127>::succ_1_type> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl_v2::hyb_vector<>, sdsl_v2::hyb_vector<>::succ_1_type> zombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl_v2::rrr_vector<127>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl_v2::hyb_vector<>> pzombit_hyb_type;
typedef sdsl_v2::rrr_vector<127> rrr_type;
typedef sdsl_v2::hyb_vector<> hyb_type;


template<class BV, class Rank>
void write_bvs(const std::string &file_name, const std::vector<BV> &bvs, const std::vector<Rank> &ranks){
    std::ofstream out(file_name);
    sdsl::write_member(bvs.size(), out);
    for(uint64_t i = 0; i < bvs.size(); ++i){
        bvs[i].serialize(out);
        ranks[i].serialize(out);
    }
    out.close();
}

template<class BV, class Rank>
void load_bvs(const std::string &file_name, std::vector<BV> &bvs, std::vector<Rank> &ranks){
    std::ifstream in(file_name);
    uint64_t size = 0;
    sdsl::read_member(size, in);
    bvs.resize(size);
    ranks.resize(size);
    for(uint64_t i = 0; i < bvs.size(); ++i){
        bvs[i].load(in);
        ranks[i].load(in);
    }
    in.close();
}

template<class BV, class Rank>
uint64_t size(const std::vector<BV> &bvs, const std::vector<Rank> &ranks){
    uint64_t size = 0;
    for(uint64_t i = 0; i < bvs.size(); ++i){
        size += sdsl::size_in_bytes(bvs[i]);
        size += sdsl::size_in_bytes(ranks[i]);
    }
    return size;
}

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

template<class BV, class Rank>
void build_bvs(const std::string &file_name, uint8_t num_bytes, std::vector<BV> &bvs, std::vector<Rank> &ranks){
    sdsl::int_vector<> text;
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
    auto sigma = map.size();
    ranks.resize(sigma);
    for(id = 1; id < sigma; ++id){
        sdsl::bit_vector aux_bv(text.size(), 0);
        for(uint j = 0; j < text.size(); ++j){
            if(text[j] == id) {
                aux_bv[j] = 1;
            }
        }
        bvs.emplace_back(BV(aux_bv));
        sdsl::util::init_support(ranks[id], &bvs[id-1]);
    }
}


template <class BV>
int run(std::string file_name, std::string index_file, uint num_bytes) {


    std::vector<BV> bvs;
    std::vector<typename BV::rank_1_type> ranks;
    std::cout << "Reading BVS ... " << std::flush;
    if(!::util::file::file_exists(index_file)){
        std::cout << "[fail]" << std::endl;
        sdsl::int_vector<> bwt;
        std::cout << "Building BVS ... " << std::flush;
        build_bvs(file_name, num_bytes,bvs, ranks);
        write_bvs(index_file, bvs, ranks);
        std::cout << "[done]" << std::endl;
    }else{
        load_bvs(index_file, bvs, ranks);
        std::cout << "[done]" << std::endl;
    }
    std::cout << "Size in Bytes : " << size(bvs, ranks) << std::endl;
    return 1;

}

int main(int argc, char** argv)
{
    std::string file_name = argv[1];
    uint num_bytes = atoi(argv[2]);

    std::cout << "---- HYB ----" << std::endl;
    std::string index_file = file_name + ".hyb.bvs";
    run<hyb_type>(file_name, index_file, num_bytes);

    index_file = file_name + ".rrr.bvs";
    std::cout << "---- RRR ----" << std::endl;
    run<rrr_type>(file_name, index_file, num_bytes);

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit.bvs";
    run<zombit_plain_type>(file_name, index_file, num_bytes);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr.bvs";
    run<zombit_rrr_type>(file_name, index_file, num_bytes);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb.bvs";
    run<zombit_hyb_type>(file_name, index_file, num_bytes);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit.bvs";
    run<pzombit_plain_type>(file_name, index_file, num_bytes);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr.bvs";
    run<pzombit_rrr_type>(file_name, index_file, num_bytes);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb.bvs";
    run<pzombit_hyb_type>(file_name, index_file, num_bytes);

}