//
// Created by Adrián on 27/2/24.
//

#include <file.hpp>
#include <sstream>
#include <sdsl/int_vector.hpp>

#include <sdsl/suffix_arrays.hpp>
#include <zombit_vector_v3.hpp>
#include <zombit_vector_v4.hpp>
#include <partitioned_zombit_vector.hpp>
#include <rec_partitioned_zombit_vector.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type; //original
typedef runs_vectors::zombit_vector_v4<sdsl::bit_vector> zombit_v2_plain_type; //version sparse
typedef runs_vectors::succ_support_zombit_v3_naive zombit_plain_succ_naive_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<127>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::rec_partitioned_zombit_vector<> rec_pzombit_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_naive pzombit_plain_succ_naive_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<127>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;
typedef sdsl::rrr_vector<127> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;
typedef sdsl::sd_vector<> sd_type;

template<class BV>
void build_bitmap(BV &bitmap, const std::string &file){

    sdsl::bit_vector aux;
    {
        std::ifstream in(file);
        std::string line;
        std::vector<uint64_t> values;
        while (std::getline(in, line)) {
            std::stringstream ss(line);
            std::string str;
            while (std::getline(ss, str, ',')) {
                values.emplace_back(std::atoll(str.c_str()));
            }
        }
        in.close();

        if(values.empty()){
            aux = sdsl::bit_vector(0);
        }else{
            uint64_t size = values.back()+1;
            aux = sdsl::bit_vector(size, 0);
            for(const auto &v : values){
                aux[v] = 1;
            }
        }
    }
   // std::cout << aux.size() << std::endl;
   // std::cout << file << std::endl;
    bitmap = BV(aux);
   // std::cout << " done." << std::endl;

}

template<class BV>
void build_bitmaps(std::vector<BV> &bitmaps, const std::string &folder){
    auto files = util::file::read_directory(folder, ".txt");
    bitmaps.resize(files.size());
    uint64_t i = 0;
    for(const auto &f : files){
        std::string path = folder + "/" + f;
        build_bitmap(bitmaps[i], path);
        ++i;
    }
}

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

template <class BV>
void run(const std::string &folder, const std::string &type){

    std::vector<BV> bitmaps;
    std::string index_name = folder + "/" + folder + ".bvs." + type;
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
        build_bitmaps(bitmaps, folder);
        //sdsl::construct_im(wm_bv, bwt.data(), 1);
        store_bitmaps(bitmaps, index_name);
    }
    std::cout << " [done]." << std::endl;
    std::cout << "Size in Bytes : " << sdsl::size_in_bytes(bitmaps) << std::endl;
    std::ofstream out(index_name + ".html");
    sdsl::write_structure<sdsl::HTML_FORMAT>(bitmaps, out);
}

int main(int argc, char** argv){
    std::string folder = argv[1];
    std::string type = argv[2];
    if(type == "zombit-plain"){
        run<zombit_plain_type>(folder, type);
    }else if (type == "zombit-rrr"){
        run<zombit_rrr_type>(folder, type);
    }else if (type == "zombit-hyb") {
        run<zombit_hyb_type>(folder, type);
    }else if(type == "zombit-v2-plain"){
        run<zombit_v2_plain_type>(folder, type);
    }else if (type == "pzombit-plain"){
        run<pzombit_plain_type>(folder, type);
    }else if (type == "pzombit-rrr"){
        run<pzombit_rrr_type>(folder, type);
    }else if (type == "pzombit-hyb"){
        run<pzombit_hyb_type>(folder, type);
    }else if (type == "hyb"){
        run<hyb_type>(folder, type);
    }else if (type == "rrr"){
        run<rrr_type>(folder, type);
    }else if (type == "sd"){
        run<sd_type>(folder, type);
    }
}