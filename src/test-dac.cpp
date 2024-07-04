//
// Created by Adrián on 7/2/24.
//


#include "dac_vector.hpp"
#include "sdsl/dac_vector.hpp"

int main(int argc, char** argv) {

    typedef uint64_t value_type;
    for (int i = 0; i < 1; ++i) {
        std::cout << "random uniform test " << i << std::endl;
        std::vector<value_type> vec;
        std::mt19937_64 rng;
        {
            std::uniform_int_distribution<uint64_t> distribution(0, 100000000);
            auto dice = bind(distribution, rng);
            for (size_t i=0; i < 100000; ++i)
                vec.push_back(dice());
        }
        sdsl::dac_vector_dp<> dac_old(vec);
        sdsl::dac_vector_dp_opt<> dac_opt(vec);
        std::cout << "Old: " << sdsl::size_in_bytes(dac_old) << std::endl;
        std::cout << "Opt: " << sdsl::size_in_bytes(dac_opt) << std::endl;

        sdsl::store_to_file(dac_old, "dac_old.txt");
        sdsl::store_to_file(dac_opt, "dac_opt.txt");
    }

    for (int i = 0; i < 1; ++i) {
        std::cout << "random exponential test " << i << std::endl;
        std::vector<value_type> vec;
        std::mt19937_64 rng;
        {
            std::exponential_distribution<double> dist(3.5);
            auto dice = bind(dist, rng);
            for (size_t i = 0; i < 100000; ++i)
                vec.push_back(uint64_t(dice() * 1000000));
        }
        sdsl::dac_vector_dp<> dac_old(vec);
        sdsl::dac_vector_dp_opt<> dac_opt(vec);
        std::cout << "Old: " << sdsl::size_in_bytes(dac_old) << std::endl;
        std::cout << "Opt: " << sdsl::size_in_bytes(dac_opt) << std::endl;

        sdsl::store_to_file(dac_old, "dac_old2.txt");
        sdsl::store_to_file(dac_opt, "dac_opt2.txt");
    }

}