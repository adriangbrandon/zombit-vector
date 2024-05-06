//
// Created by Adrián on 30/4/24.
//
#include <random>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <file.hpp>

std::vector<uint64_t> queries(uint64_t max, uint64_t size){
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, max);
    auto dice = bind(distribution, rng);
    std::vector<uint64_t> res;
    while(res.size() < size){
        res.push_back(dice());
    }
    return res;
}

void help(const std::string &ex){
    std::cout << ex << " <max> <size> <file>" << std::endl;
}

int main(int argc, char* argv[]) {

    if(argc != 4){
        help(argv[0]);
        return 0;
    }
    uint64_t max = std::atoll(argv[1]);
    uint64_t size = std::atoll(argv[2]);
    auto res = queries(max, size);
    util::file::write_to_file(argv[3], res);

}