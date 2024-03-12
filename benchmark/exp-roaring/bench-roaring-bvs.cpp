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

#define TIMES 5

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

template<class BV, class Succ>
uint64_t load_bitmaps(std::vector<BV> &bitmaps, std::vector<Succ> &succs, const std::string &name){
    std::ifstream f(name);
    if(!f.good()) return false;
    sdsl::load(bitmaps, f);
    sdsl::load(succs, f);
    uint64_t min = UINT64_MAX;
    for(uint64_t i = 0; i < succs.size(); ++i){
        succs[i].set_vector(&bitmaps[i]);
        if(bitmaps[i].size() < min) min = bitmaps[i].size();
    }
    f.close();
    return min;
}

template <class BV, class Succ>
void run(const std::string &path, const std::string &type, const std::vector<uint64_t> &qrandom){

    std::vector<BV> bitmaps;
    std::vector<Succ> succs;

    std::string folder = util::file::remove_path(path);
    std::string index_name = path + "/bvs/" + folder + ".bvs." + type;
    std::cout << "Reading BVS... " << std::flush;
    auto max = load_bitmaps(bitmaps, succs, index_name);
    if(!max){
        std::cout << " [fail]." << std::endl;
        exit(0);
    }
    std::cout << " [done]." << std::endl;
    auto bytes = sdsl::size_in_bytes(bitmaps) + sdsl::size_in_bytes(succs);

    std::cout << "--- Membership queries ---" << std::endl;
    uint64_t sum = 0;
    double avg = 0;
    std::vector<uint64_t> mem_queries;
    mem_queries.reserve(qrandom.size());
    for(auto i : qrandom){
        mem_queries.emplace_back(i % max);
    }

    for(uint64_t k = 0; k < bitmaps.size(); ++k){
        for(uint64_t i = 0; i < mem_queries.size(); ++i){
            sum += bitmaps[k][mem_queries[i]];
        }
    }

    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t k = 0; k < bitmaps.size(); ++k){
            for(uint64_t i = 0; i < mem_queries.size(); ++i){
                sum += bitmaps[k][mem_queries[i]];
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_list = time / (double) bitmaps.size();
        avg += per_list;
        std::cout << "Membership " << time << " ns (per list: " << per_list << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_member = avg / TIMES;
    std::cout << "Membership avg per list: " << avg_member << " ns." << std::endl;
    std::cout << std::endl;
    sum = 0; avg = 0;

    std::cout << "--- Sequential queries ---" << std::endl;

    for(uint64_t k = 0; k < bitmaps.size(); ++k){
        uint64_t i = 0;
        while(i < bitmaps[k].size()){
            i = succs[k](i);
            ++i;
        }
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t k = 0; k < bitmaps.size(); ++k){
            uint64_t i = 0;
            while(i < bitmaps[k].size()){
                i = succs[k](i);
                sum += i;
                ++i;
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_query = time / (double) bitmaps.size();
        avg += per_query;
        std::cout << "Sequential " << time << " ns (per query: " << per_query << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_sequential = avg / TIMES;
    std::cout << "Sequential avg per list: " << avg_sequential << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    std::cout << "--- Intersection queries ---" << std::endl;

    for(uint64_t k = 0; k < bitmaps.size(); ++k){
        uint64_t i = 0;
        while(i < bitmaps[k].size()){
            i = succs[k](i);
            ++i;
        }
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t k = 0; k < bitmaps.size()-1; ++k){
            uint64_t c = 0, prev_c = -1, c_l, l = k;
            while(true){
                c_l = succs[l](c);
                if(c_l >= bitmaps[l].size()) break;
                if(c_l == prev_c) sum += c_l;
                c = prev_c = c_l;
                l = (l == k) ? k+1 : k;
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        auto per_list = time / (double) (bitmaps.size()-1);
        avg += per_list;
        std::cout << "Intersection " << time << " ns (per operation: " << per_list << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_intersection = avg / TIMES;
    std::cout << "Sequential avg per list: " << avg_intersection << " ns." << std::endl;
    std::cout << std::endl;

    std::cerr << bytes << ";" << avg_member << ";" << avg_sequential << ";" << avg_intersection << std::endl;
}

int main(int argc, char** argv){
    std::string folder = argv[1];

    auto random = queries(100000, 1000);
    run<hyb_type, typename hyb_type::succ_1_type>(folder, "hyb", random);
    run<rrr_type, typename rrr_type::succ_1_type>(folder, "rrr", random);
    run<sd_type, sd_succ_type>(folder, "sd", random);
    run<oz_type, typename oz_type::succ_1_type>(folder, "oz", random);
    run<zombit_plain_type, zombit_plain_succ_naive_type>(folder, "zombit-plain", random);
    run<zombit_v2_plain_type, zombit_v2_plain_succ_naive_type>(folder, "zombit-s-plain", random);
    run<pzombit_plain_type, pzombit_plain_succ_naive_type>(folder, "pzombit-plain", random);
    run<pzombit_sparse_plain_type, pzombit_sparse_plain_succ_naive_type>(folder, "pzombit-s-plain", random);


}