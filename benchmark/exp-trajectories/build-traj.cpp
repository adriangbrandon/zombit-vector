//
// Created by Adrián on 5/6/24.
//

#include <sdsl/bit_vectors.hpp>

void store_bitmap(std::vector<uint64_t> &times, const std::string &file){
    uint64_t max = times.back();
    sdsl::bit_vector bv(max+1, 0);
    for(uint64_t t : times){
        bv[t] = 1;
    }
    sdsl::store_to_file(bv, file);
}


int main(int argc, char** argv){

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    uint limit = std::atoi(argv[3]);

    std::ifstream in(input_file);
    uint64_t id, t, x, y, prev_id = -1ULL;
    std::vector<uint64_t> times;
    while(true){
        in >> id >> t >> x >> y;
        if(in.eof() || id == limit ) break;
        if(prev_id != -1ULL && prev_id != id){
            store_bitmap(times, output_file + "." + std::to_string(prev_id));
            times.clear();
        }else{
            times.push_back(t);
        }
        prev_id = id;
    }
    store_bitmap(times, output_file + std::to_string(prev_id));
    in.close();
}