//
// Created by Adrián on 7/5/24.
//

#include <iostream>
#include <sdsl/bit_vectors.hpp>

int main(int argc, char* argv[])
{
    if(argc != 3){
        std::cout << argv[0] << " <input> <output>" << std::endl;
        return 0;
    }
    std::string input = argv[1];
    std::string output = argv[2];
    sdsl::bit_vector bv;
    if(!sdsl::load_from_file(bv, input)){
        std::cout << "Error: " << input << " does not exist." << std::endl;
        exit(0);
    };
    std::ofstream out(output);
    bool comma = false;
    for(uint64_t i = 0; i < bv.size(); ++i){
        if(bv[i]) {
            if(comma) out << ",";
            out << i;
            comma = true;
        }
    }
    out << bv.size();
    out.close();
}