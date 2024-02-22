//
// Created by Adrián on 21/2/24.
//

#include <string>
#include <sdsl/suffix_arrays.hpp>
#include <iostream>

void build_bwt(const sdsl::int_vector<8> &text, sdsl::int_vector<8> &bwt){
    sdsl::int_vector<> sa;
    std::cout << "Sorting suffixes... " << std::flush;
    sdsl::qsufsort::construct_sa(sa, text);
    std::cout << " [done]." << std::endl;
    std::cout << "Building BWT... " << std::flush;
    for (uint64_t i = 0; i < sa.size(); ++i) {
        if (sa[i] == 0){
            bwt[i] = text[text.size()-1];
        }else{
            bwt[i] = text[sa[i]-1];
        }
    }
    std::cout << " [done]." << std::endl;
}

void read_file(const std::string &file_name, sdsl::int_vector<8> &text){

    std::cout << "Reading file... " << std::flush;
    sdsl::load_vector_from_file(text, file_name);
    std::cout << " [done]." << std::endl;
    std::cout << "Mapping alphabet... " << std::flush;
    uint8_t id = 1;
    std::unordered_map<uint8_t, uint8_t > map;
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
    std::cout << " [done]." << std::endl;
}

void run(const std::string &file_name){

    sdsl::int_vector<8> text, bwt;
    read_file(file_name, text);
    build_bwt(text, bwt);

    std::string bwt_file = file_name + ".bwt";
    sdsl::store_to_file(bwt, bwt_file);

}

int main(int argc, char** argv){
    std::string file_name = argv[1];
    run(file_name);
}