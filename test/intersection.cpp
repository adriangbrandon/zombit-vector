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
// Created by Adrián on 5/1/23.
//

#include <zombit_vector_v4.hpp>
#include <zombit_vector_v3.hpp>
#include <partitioned_zombit_vector.hpp>
#include <partitioned_zombit_vector_sparse.hpp>

uint64_t check_succ(uint64_t i, sdsl::bit_vector &bm){
    for(uint64_t j = i; j < bm.size(); ++j){
        if(bm[j]) return j;
    }
    return bm.size();
}

//ratio:
sdsl::bit_vector generate(uint64_t size, uint64_t ratio){

    sdsl::bit_vector bv = sdsl::bit_vector(size, 0);
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, bv.size()-1);
    auto dice = bind(distribution, rng);
    // populate vectors with some other bits
    for (uint64_t i=0; i < bv.size()/ratio; ++i) {
        uint64_t x = dice();
        bv[x] = 1;
    }
    return bv;
}


sdsl::bit_vector generate_runs(uint64_t size, double mean_1, double stdev_1, double mean_0, double stdev_0) {

    sdsl::bit_vector bm(size);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    //Error con seed=1380572158; distribution_zeroes (1000, 100); distribution_ones (100, 10);
    //unsigned seed = 1380572158;
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution_zeroes(mean_0, stdev_0);
    std::normal_distribution<double> distribution_ones(mean_1, stdev_1);

    auto index = 0;
    bool value = false;
    while (index < bm.size()) {
        int64_t run_size = 0;
        if (value) {
            run_size = std::abs(static_cast<int64_t >(std::ceil(distribution_ones(generator))));
        } else {
            run_size = std::abs(static_cast<int64_t >(std::ceil(distribution_zeroes(generator))));
        }
        //std::cout << "run_size: " << run_size << std::endl;
        for (uint64_t i = 0; i < run_size; ++i) {
            bm[index] = value;
            ++index;
            if (index >= bm.size()) break;
        }
        value = !value;
    }
    return bm;
}

void test() {

    std::vector<uint64_t> b1_ones = {1, 8, 300, 350, 800, 1232, 1233, 1234, 1456, 1457, 1458, 1459, 1499};
    std::vector<uint64_t> b2_ones = {2, 8, 322, 323, 324, 325, 350, 800, 1000, 1233, 1234, 1235, 1459, 1499};

    /*runs_vectors::zombit_vector_v4<sdsl::bit_vector> pz(bm);
    runs_vectors::succ_support_zombit_v4_naive pz_succ;
    runs_vectors::zombit_vector_v4<sdsl::bit_vector>::rank_1_type pz_rank;*/

   /*runs_vectors::partitioned_zombit_vector<sdsl::bit_vector> pz(bm);
    runs_vectors::succ_support_partitioned_zombit_naive pz_succ;
    runs_vectors::partitioned_zombit_vector<sdsl::bit_vector>::rank_1_type pz_rank;*/

    sdsl::bit_vector b1(1500, 0);
    sdsl::bit_vector b2(1500, 0);

    for(const auto &o : b1_ones){
        b1[o] = 1;
    }
    for(const auto &o : b2_ones){
        b2[o] = 1;
    }

    std::vector<runs_vectors::zombit_vector_v3<sdsl::bit_vector>> pz(2);
    pz[0] = runs_vectors::zombit_vector_v3<sdsl::bit_vector>(b1);
    pz[1] = runs_vectors::zombit_vector_v3<sdsl::bit_vector>(b2);

    std::vector<runs_vectors::zombit_iterator_v3> itrs(2);
    itrs[0] = runs_vectors::zombit_iterator_v3(&pz[0]);
    itrs[1] = runs_vectors::zombit_iterator_v3(&pz[1]);

    uint64_t i = 0, c = 0, prev = -1, cnt = 0;
    while(itrs[i].next(c)){
        c = *itrs[i];
        if(c == prev) ++cnt;
        else cnt = 1;
        i = (i+1) % 2;
        prev = c;
        if(cnt == 2){
            std::cout << c << std::endl;
            ++c;
        }

    }



}

int main(int argc, char** argv) {

    test();
}