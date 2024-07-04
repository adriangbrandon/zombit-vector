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

namespace pef {

    typedef uint32_t posting_t ;
    typedef uint64_t cost_t;

    struct optimal_partition {

        std::vector<posting_t> partition;
        cost_t cost_opt = 0; // the costs are in bits!

        template <typename ForwardIterator>
        struct cost_window {
            // a window reppresent the cost of the interval [start, end)

            ForwardIterator start_it;
            ForwardIterator end_it;
            // starting and ending position of the window
            posting_t start = 0;
            posting_t end = 0; // end-th position is not in the current window
            posting_t min_p = 0; // element that preceed the first element of the window
            posting_t max_p = 0;

            cost_t cost_upper_bound; // The maximum cost for this window

            cost_window(ForwardIterator begin, cost_t cost_upper_bound)
                    : start_it(begin)
                    , end_it(begin)
                    , min_p(*begin)
                    , max_p(0)
                    , cost_upper_bound(cost_upper_bound)
            {}

            uint64_t universe() const
            {
                return max_p - min_p + 1;
            }

            uint64_t size() const
            {
                return end - start;
            }

            void advance_start()
            {
                min_p = *start_it + 1;
                ++start;
                ++start_it;
            }

            void advance_end()
            {
                max_p = *end_it;
                ++end;
                ++end_it;
            }

        };

        optimal_partition()
        {}

        template <typename ForwardIterator, typename CostFunction>
        optimal_partition(ForwardIterator begin, uint64_t universe, uint64_t size,
                          CostFunction cost_fun, double eps1, double eps2)
        {
            cost_t single_block_cost = cost_fun(universe, size);
            std::vector<cost_t> min_cost(size+1, single_block_cost);
            min_cost[0] = 0;

            // create the required window: one for each power of approx_factor
            std::vector<cost_window<ForwardIterator>> windows;
            cost_t cost_lb = cost_fun(1, 1); // minimum cost
            cost_t cost_bound = cost_lb;
            while (eps1 == 0 || cost_bound < cost_lb / eps1) {
                windows.emplace_back(begin, cost_bound);
                if (cost_bound >= single_block_cost) break;
                cost_bound = cost_bound * (1 + eps2);
            }

            std::vector<posting_t> path(size + 1, 0);
            for (posting_t i = 0; i < size; i++) {
                size_t last_end = i + 1;
                for (auto& window: windows) {

                    assert(window.start == i);
                    while (window.end < last_end) {
                        window.advance_end();
                    }

                    cost_t window_cost;
                    while (true) {
                        window_cost = cost_fun(window.universe(), window.size());
                        if ((min_cost[i] + window_cost < min_cost[window.end])) {
                            min_cost[window.end] = min_cost[i] + window_cost;
                            path[window.end] = i;
                        }
                        last_end = window.end;
                        if (window.end == size) break;
                        if (window_cost >= window.cost_upper_bound) break;
                        window.advance_end();
                    }

                    window.advance_start();
                }
            }

            posting_t curr_pos = size;
            while( curr_pos != 0 ) {
                partition.push_back(curr_pos);
                curr_pos = path[curr_pos];
            }
            std::reverse(partition.begin(), partition.end());
            cost_opt = min_cost[size];
        }
    };

}

#endif //SUCC_VECTORS_OPTIMAL_PARTITION_HPP
