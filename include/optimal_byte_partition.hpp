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
// Created by Adrián on 26/12/22.
//

#ifndef SUCC_VECTORS_OPTIMAL_PARTITION_HPP
#define SUCC_VECTORS_OPTIMAL_PARTITION_HPP

namespace partition {

    typedef uint32_t pos_t ;
    typedef uint64_t cost_t;
    enum block_t {run0, run1, mixed};

    struct optimal_variable {

        std::vector<pos_t> partition;
        cost_t cost_opt = 0; // the costs are in bits!

        template<class Iterator>
        struct cost_byte_window {
            Iterator beg_it;
            Iterator end_it;
            // starting and ending position of the window
            uint64_t beg = 0;
            uint64_t end = 0;
            uint64_t cnt_run0 = 0;
            uint64_t cnt_run1 = 0;
            uint64_t cnt_mixed = 0;

            cost_t cost_upper_bound; // The maximum cost for this window

            cost_byte_window(Iterator begin, cost_t cost_upper_bound)
            : beg_it(begin)
            , end_it(begin)
            , cost_upper_bound(cost_upper_bound)
            { }


            uint64_t size() const
            {
                return end - beg;
            }

            bool is_mixed() const
            {
                return (cnt_mixed > 0) || (cnt_run1 > 0 && cnt_run0 > 0);
            }

            void advance_start()
            {
                if(*beg_it == block_t::mixed){
                    --cnt_mixed;
                }else if(*beg_it == block_t::run0){
                    --cnt_run0;
                }else{
                    --cnt_run1;
                }
                ++beg_it;
                ++beg;
            }

            void advance_end()
            {
                if(*end_it == block_t::mixed){
                    ++cnt_mixed;
                }else if(*end_it == block_t::run0){
                    ++cnt_run0;
                }else{
                    ++cnt_run1;
                }
                ++end_it;
                ++end;
            }


        };


        optimal_variable()
        {}

        template <typename Iterator>
        optimal_variable(Iterator begin, uint64_t blocks, uint64_t size,
                          double eps1 = 0.03, double eps2 = 0.3)
        {
            cost_t single_block_cost = size; //in bits
            std::vector<cost_t> min_cost(blocks+1, single_block_cost);
            min_cost[0] = 0;
            cost_t cost_length = sdsl::bits::hi(blocks)+1;
            // create the required window: one for each power of approx_factor
            std::vector<cost_byte_window<Iterator>> windows;
            cost_t cost_lb = 2 + cost_length; // minimum cost (2 + log_2(size) bits)
            cost_t cost_bound = cost_lb;
            while (eps1 == 0 || cost_bound < cost_lb / eps1) {
                windows.emplace_back(begin, cost_bound);
                if (cost_bound >= single_block_cost) break;
                cost_bound = cost_bound * (1 + eps2);
            }

            std::vector<pos_t> path(blocks + 1, 0);
            for (pos_t i = 0; i < blocks; i++) {
                size_t last_end = i + 1;
                for (auto& window: windows) {

                    assert(window.beg == i);
                    while (window.end < last_end) {
                        window.advance_end();
                    }

                    cost_t window_cost;
                    while (true) {
                        if(window.is_mixed()){
                            window_cost = 2 + 2*cost_length + window.size()*8;
                        }else{
                            window_cost = 2 + cost_length;
                        }

                        if ((min_cost[i] + window_cost < min_cost[window.end])) {
                            min_cost[window.end] = min_cost[i] + window_cost;
                            path[window.end] = i;
                        }
                        last_end = window.end;
                        if (window.end == blocks) break;
                        if (window_cost >= window.cost_upper_bound) break;
                        window.advance_end();
                    }

                    window.advance_start();
                }
            }

            pos_t curr_pos = blocks;
            while( curr_pos != 0 ) {
                partition.push_back(curr_pos);
                curr_pos = path[curr_pos];
            }
            std::reverse(partition.begin(), partition.end());
            cost_opt = min_cost[blocks];
        }
    };

}

#endif //SUCC_VECTORS_OPTIMAL_PARTITION_HPP
