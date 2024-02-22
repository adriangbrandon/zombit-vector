//
// Created by Adrián on 21/2/24.
//

#include <string>
#include <sdsl/suffix_arrays.hpp>
#include <iostream>

void build_bwt(const sdsl::int_vector<> &text, sdsl::int_vector<> &bwt){
    sdsl::int_vector<> sa;
    sdsl::qsufsort::construct_sa(sa, text);

    for (uint64_t i = 0; i < sa.size(); ++i) {
        if (sa[i] == 0){
            bwt[i] = text[text.size()-1];
        }else{
            bwt[i] = text[sa[i]-1];
        }
    }
}

void read_file(const std::string &file_name, sdsl::int_vector<> &text){
    sdsl::load_vector_from_file(text, file_name);
    uint64_t id = 1;
    std::unordered_map<uint64_t, uint64_t > map;
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
}

void run(const std::string &file_name){

    sdsl::int_vector<> text, bwt;
    read_file(file_name, text);
    build_bwt(text, bwt);

    std::string bwt_file = file_name + ".bwt";
    sdsl::store_to_file(bwt, bwt_file);

}

int main(int argc, char** argv){
    std::string file_name = argv[1];
    run(file_name);
}