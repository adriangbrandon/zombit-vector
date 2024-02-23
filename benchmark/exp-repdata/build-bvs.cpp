//
// Created by Adrián on 21/2/24.
//

#include <string>
#include <vector>
#include <iostream>
#include <sdsl/suffix_arrays.hpp>
#include <zombit_vector_v3.hpp>
#include <partitioned_zombit_vector.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type;
typedef runs_vectors::succ_support_zombit_v3_naive zombit_plain_succ_naive_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<127>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_naive pzombit_plain_succ_naive_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<127>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;
typedef sdsl::rrr_vector<127> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;

template<class BV>
bool load_bitmaps(std::vector<BV> &bitmaps, const std::string &name){
    std::ifstream f(name);
    if(!f.good()) return false;
    sdsl::load(bitmaps, f);
    f.close();
    return true;
}

template<class BV>
void store_bitmaps(std::vector<BV> &bitmaps, const std::string &name){
    std::ofstream f(name);
    sdsl::serialize(bitmaps, f);
    f.close();
}

template<class BV>
void construct(std::vector<BV> &bitmaps, const std::string &file){
    std::ifstream f(file);
    sdsl::int_vector_buffer<8> text_buf(file, std::ios::in, 1024*1024, 8, true);
    std::cout << "Size: " << text_buf.size() << std::endl;

    std::vector<sdsl::bit_vector> aux;
    for(uint64_t i = 0; i < text_buf.size()-1; ++i){
        auto v = text_buf[i];
        if(aux.size() < text_buf[i]){
            aux.emplace_back(sdsl::bit_vector(text_buf.size(), 0));
        }
        aux[v-1][i] = 1;
    }
    bitmaps.resize(aux.size());
    for(uint64_t i = 0; i < bitmaps.size(); ++i){
        bitmaps[i] = BV(aux[i]);
    }
}


template <class BV>
void run(const std::string &file_name, const std::string &index_name){

    std::vector<BV> bitmaps;
    std::cout << "Reading BVS... " << std::flush;
    if(!load_bitmaps(bitmaps, index_name)){
        std::cout << " [fail]." << std::endl;
        /*std::cout << "Loading bwt... " << std::flush;
        sdsl::int_vector<8> bwt;
        sdsl::load_from_file(bwt, file_name);
        std::cout << " [done]." << std::endl;
        int sigma = 0;
        for(auto &a : bwt){
            if(a > sigma) sigma = a;
        }
        std::cout << "sigma de verdade: " << sigma << std::endl;
        std::cout << bwt.size() << std::endl;
        std::cout << (uint64_t) bwt.width() << std::endl;*/
        std::cout << "Building BVS... " << std::flush;
        construct(bitmaps, file_name);
        //sdsl::construct_im(wm_bv, bwt.data(), 1);
        store_bitmaps(bitmaps, index_name);
    }
    std::cout << " [done]." << std::endl;


    std::cout << "sigma: " << bitmaps.size()+1 << std::endl;
    std::cout << "size: " << bitmaps[0].size() << std::endl;
    std::cout << "Size in Bytes : " << sdsl::size_in_bytes(bitmaps) << std::endl;
    std::ofstream out(index_name + ".html");
    sdsl::write_structure<sdsl::HTML_FORMAT>(bitmaps, out);
}

int main(int argc, char** argv){
    std::string file_name = argv[1];
    std::string index_name = argv[2];
    std::string type = argv[3];
    if(type == "zombit-plain"){
        run<zombit_plain_type>(file_name, index_name);
    }else if (type == "zombit-rrr"){
        run<zombit_rrr_type>(file_name, index_name);
    }else if (type == "zombit-hyb"){
        run<zombit_hyb_type>(file_name, index_name);
    }else if (type == "pzombit-plain"){
        run<pzombit_plain_type>(file_name, index_name);
    }else if (type == "pzombit-rrr"){
        run<pzombit_rrr_type>(file_name, index_name);
    }else if (type == "pzombit-hyb"){
        run<pzombit_hyb_type>(file_name, index_name);
    }else if (type == "hyb"){
        run<hyb_type>(file_name, index_name);
    }else if (type == "rrr"){
        run<rrr_type>(file_name, index_name);
    }
}