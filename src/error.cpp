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
// Created by Adrián on 2/1/23.
//
#include <time.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support.hpp>
#include <zombit_vector_v3.hpp>
#include <partitioned_zombit_vector.hpp>

int main(int argc, char** argv) {

    sdsl::bit_vector bv;
    sdsl::load_from_file(bv, "bit-vector-exp2.notequal.100000000.10000.250.bin");

    sdsl::rank_support_v<1> rank_bv;
    sdsl::succ_support_v<1> succ_bv;
    sdsl::util::init_support(rank_bv, &bv);
    sdsl::util::init_support(succ_bv, &bv);

    //typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_type;
    typedef runs_vectors::partitioned_zombit_vector<sdsl::bit_vector> zombit_type;
    typedef typename zombit_type::rank_1_type rank_zombit_type;
    typedef typename zombit_type::succ_1_type succ_zombit_type;
    {
        zombit_type zombit(bv);
        rank_zombit_type rank_zombit;
        succ_zombit_type succ_zombit;
        sdsl::util::init_support(rank_zombit, &zombit);
        sdsl::util::init_support(succ_zombit, &zombit);

        std::cout << "==== In memory ====" << std::endl;
        for (uint64_t i = 0; i < bv.size(); ++i) {
            if (rank_zombit(i + 1) != rank_bv(i + 1)) {
                std::cout << "Error [i=" << i << "] " << std::endl;
                std::cout << "BV rank= " << rank_bv(i + 1) << std::endl;
                std::cout << "ZB rank= " << rank_zombit(i + 1) << std::endl;
                return 0;
            }
        }
        std::cout << "Rank: Everything is ok!" << std::endl;

        for (uint64_t i = 0; i < bv.size(); ++i) {
            if (succ_zombit(i) != succ_bv(i)) {
                std::cout << "Error [i=" << i << "] " << std::endl;
                std::cout << "BV succ= " << succ_bv(i) << std::endl;
                std::cout << "ZB succ= " << succ_zombit(i) << std::endl;
                return 0;
            }
        }
        std::cout << "Succ: Everything is ok!" << std::endl;
        std::cout << "===================" << std::endl;
        std::cout << std::endl;

        sdsl::store_to_file(zombit, "zombit");
        sdsl::store_to_file(rank_zombit, "rank_zombit");
        sdsl::store_to_file(succ_zombit, "succ_zombit");
    }

    {
        zombit_type zombit;
        rank_zombit_type rank_zombit;
        succ_zombit_type succ_zombit;

        sdsl::load_from_file(zombit, "zombit");
        std::cout << "a" << std::endl;
        sdsl::load_from_file(rank_zombit, "rank_zombit");
        std::cout << "a" << std::endl;
        sdsl::load_from_file(succ_zombit, "succ_zombit");
        std::cout << "a" << std::endl;
        rank_zombit.set_vector(&zombit);
        std::cout << "a" << std::endl;
        succ_zombit.set_vector(&zombit);
        std::cout << "a" << std::endl;

        std::cout << "==== From disk to memory ====" << std::endl;
        for (uint64_t i = 0; i < bv.size(); ++i) {
            if (rank_zombit(i + 1) != rank_bv(i + 1)) {
                std::cout << "Error [i=" << i << "] " << std::endl;
                std::cout << "BV rank= " << rank_bv(i + 1) << std::endl;
                std::cout << "ZB rank= " << rank_zombit(i + 1) << std::endl;
                return 0;
            }
        }
        std::cout << "Rank: Everything is ok!" << std::endl;

        for (uint64_t i = 0; i < bv.size(); ++i) {
            if (succ_zombit(i) != succ_bv(i)) {
                std::cout << "Error [i=" << i << "] " << std::endl;
                std::cout << "BV succ= " << succ_bv(i) << std::endl;
                std::cout << "ZB succ= " << succ_zombit(i) << std::endl;
                return 0;
            }
        }
        std::cout << "Succ: Everything is ok!" << std::endl;
        std::cout << "===================" << std::endl;
        std::cout << std::endl;
    }

}