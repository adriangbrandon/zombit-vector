//
// Created by Adrián on 12/2/24.
//

#include <sdsl/bit_vectors.hpp>


int main(int argc, char** argv)
{
    std::string file_name = argv[1];
    sdsl::bit_vector bv;
    if(!sdsl::load_from_file(bv, file_name)){
        std::cout << "Error: " << file_name << " does not exist." << std::endl;
        exit(0);
    };

    bool c = bv[0];
    uint64_t run = 1;
    for(uint64_t i = 1; i < bv.size(); ++i){
        if(c != bv[i]){
            ++run;
            c = bv[i];
        }
    }

    std::cerr << bv.size() << ";" << run << std::endl;

}