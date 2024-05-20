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
#include <partitioned_zombit_vector_sparse.hpp>
#include <succ_support_sd.hpp>
#include <oz_vector.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type;
typedef runs_vectors::succ_support_zombit_v3_naive zombit_plain_succ_naive_type;
typedef typename runs_vectors::zombit_vector_v3<sdsl::bit_vector>::rank_1_type zombit_plain_rank_type;
typedef runs_vectors::zombit_vector_v4<sdsl::bit_vector> zombit_v2_plain_type;
typedef runs_vectors::succ_support_zombit_v4_naive zombit_v2_plain_succ_naive_type;
typedef typename runs_vectors::zombit_vector_v4<sdsl::bit_vector>::rank_1_type zombit_v2_plain_rank_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<15>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::zombit_vector_v4<sdsl::rrr_vector<15>> zombit_v2_rrr_type;
typedef runs_vectors::zombit_vector_v4<sdsl::hyb_vector<>> zombit_v2_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_naive pzombit_plain_succ_naive_type;
typedef typename runs_vectors::partitioned_zombit_vector<>::rank_1_type pzombit_plain_rank_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<15>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;

typedef runs_vectors::partitioned_zombit_vector_sparse<> pzombit_sparse_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_sparse_naive pzombit_sparse_plain_succ_naive_type;
typedef typename runs_vectors::partitioned_zombit_vector_sparse<>::rank_1_type pzombit_sparse_plain_rank_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<sdsl::rrr_vector<15>> pzombit_sparse_rrr_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<sdsl::hyb_vector<>> pzombit_sparse_hyb_type;

typedef sdsl::rrr_vector<15> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;
typedef sdsl::sd_vector<> sd_type;
typedef sdsl::sd_vector<>::rank_1_type sd_rank_type;
typedef sdsl::succ_support_sd<> sd_succ_type;
typedef runs_vectors::oz_vector<> oz_type;


template<class BV, class Succ>
void build_bitmap(BV &bitmap, Succ &succ, const std::string &file){

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
    sdsl::util::init_support(succ, &bitmap);
   // std::cout << " done." << std::endl;

}


uint64_t get_number_file(const std::string  &s1){
    uint64_t p1 = 0;
    uint64_t units = 0;
    for(int64_t i = s1.size(); i >= 0; --i){
        if(isdigit(s1[i])){
            units = std::max(1UL, units*10);
            p1 += (s1[i]-'0') * units;
        }else{
            if(units) break;
        }
    }
    return p1;
}

bool compare_file(const std::string &s1, const std::string &s2){
    uint64_t p1 = get_number_file(s1);
    uint64_t p2 = get_number_file(s2);
    return p1 < p2;

}


template<class BV, class Succ>
void build_bitmaps(std::vector<BV> &bitmaps, std::vector<Succ> &succs, const std::string &folder){
    auto files = util::file::read_directory(folder, ".txt");
    bitmaps.resize(files.size());
    succs.resize(files.size());

    //Sort correctly the files
    std::sort(files.begin(), files.end(), compare_file);

    uint64_t i = 0;
    for(const auto &f : files){
        std::string path = folder + "/" + f;
        std::cout << path << std::endl;
        build_bitmap(bitmaps[i], succs[i], path);
        ++i;
    }
}

template<class BV, class Succ>
bool load_bitmaps(std::vector<BV> &bitmaps, std::vector<Succ> &succs, const std::string &name){
    std::ifstream f(name);
    if(!f.good()) return false;
    sdsl::load(bitmaps, f);
    sdsl::load(succs, f);
    for(uint64_t i = 0; i < succs.size(); ++i){
        succs[i].set_vector(&bitmaps[i]);
    }
    f.close();
    return true;
}

template<class BV, class Succ>
void store_bitmaps(std::vector<BV> &bitmaps, std::vector<Succ> &succs, const std::string &name){
    std::ofstream f(name);
    sdsl::serialize(bitmaps, f);
    sdsl::serialize(succs, f);
    f.close();
}

template <class BV, class Succ>
void run(const std::string &path, const std::string &type){

    std::vector<BV> bitmaps;
    std::vector<Succ> succs;

    std::string folder = util::file::remove_path(path);
    std::string index_name = path + "/bvs/" + folder + ".bvs." + type;
    std::cout << "Reading BVS... " << std::flush;
    if(true or !load_bitmaps(bitmaps, succs, index_name)){
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
        build_bitmaps(bitmaps, succs, path);
        //sdsl::construct_im(wm_bv, bwt.data(), 1);
        store_bitmaps(bitmaps, succs, index_name);
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
        run<zombit_plain_type, zombit_plain_succ_naive_type>(folder, type);
    }else if (type == "zombit-rrr"){
        run<zombit_rrr_type, typename zombit_rrr_type::succ_1_type>(folder, type);
    }else if (type == "zombit-hyb") {
        run<zombit_hyb_type,  typename zombit_hyb_type::succ_1_type>(folder, type);
    }else if(type == "zombit-s-plain"){
        run<zombit_v2_plain_type, zombit_v2_plain_succ_naive_type>(folder, type);
    }else if (type == "pzombit-plain"){
        run<pzombit_plain_type, pzombit_plain_succ_naive_type>(folder, type);
    }else if (type == "pzombit-s-plain"){
        run<pzombit_sparse_plain_type, pzombit_sparse_plain_succ_naive_type>(folder, type);
    }else if (type == "hyb"){
        run<hyb_type, typename hyb_type::succ_1_type>(folder, type);
    }else if (type == "rrr"){
        run<rrr_type, typename rrr_type::succ_1_type>(folder, type);
    }else if (type == "sd"){
        run<sd_type, sd_succ_type>(folder, type);
    }else if (type == "oz"){
        run<oz_type, typename oz_type::succ_1_type>(folder, type);
    }
}