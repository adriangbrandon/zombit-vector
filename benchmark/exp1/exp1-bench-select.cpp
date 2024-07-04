//
// Created by Adrián on 12/2/24.
//

#include <zombit_vector_v3.hpp>
#include <zombit_vector_v4.hpp>
#include <partitioned_zombit_vector.hpp>
#include <partitioned_zombit_vector_sparse.hpp>
#include <sdsl/hyb_vector.hpp>
#include <sdsl/rrr_vector.hpp>
#include <sdsl/sd_vector.hpp>
#include <sdsl/succ_support_sd.hpp>
#include <oz_vector.hpp>
#include <file.hpp>

typedef runs_vectors::zombit_vector_v3<sdsl::bit_vector> zombit_plain_type;
typedef runs_vectors::succ_support_zombit_v3_naive zombit_plain_succ_naive_type;
typedef typename runs_vectors::zombit_vector_v3<sdsl::bit_vector>::rank_1_type zombit_plain_rank_type;
typedef typename runs_vectors::zombit_vector_v3<sdsl::bit_vector>::select_1_type zombit_plain_select_type;
typedef runs_vectors::zombit_vector_v4<sdsl::bit_vector> zombit_v2_plain_type;
typedef runs_vectors::succ_support_zombit_v4_naive zombit_v2_plain_succ_naive_type;
typedef typename runs_vectors::zombit_vector_v4<sdsl::bit_vector>::rank_1_type zombit_v2_plain_rank_type;
typedef typename runs_vectors::zombit_vector_v4<sdsl::bit_vector>::select_1_type zombit_v2_plain_select_type;
typedef runs_vectors::zombit_vector_v3<sdsl::rrr_vector<15>> zombit_rrr_type;
typedef runs_vectors::zombit_vector_v3<sdsl::hyb_vector<>> zombit_hyb_type;
typedef runs_vectors::zombit_vector_v4<sdsl::rrr_vector<15>> zombit_v2_rrr_type;
typedef runs_vectors::zombit_vector_v4<sdsl::hyb_vector<>> zombit_v2_hyb_type;
typedef runs_vectors::partitioned_zombit_vector<> pzombit_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_naive pzombit_plain_succ_naive_type;
typedef typename runs_vectors::partitioned_zombit_vector<>::rank_1_type pzombit_plain_rank_type;
typedef typename runs_vectors::partitioned_zombit_vector<>::select_1_type pzombit_plain_select_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::rrr_vector<15>> pzombit_rrr_type;
typedef runs_vectors::partitioned_zombit_vector<sdsl::hyb_vector<>> pzombit_hyb_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<> pzombit_sparse_plain_type;
typedef runs_vectors::succ_support_partitioned_zombit_sparse_naive pzombit_sparse_plain_succ_naive_type;
typedef typename runs_vectors::partitioned_zombit_vector_sparse<>::rank_1_type pzombit_sparse_plain_rank_type;
typedef typename runs_vectors::partitioned_zombit_vector_sparse<>::select_1_type pzombit_sparse_plain_select_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<sdsl::rrr_vector<15>> pzombit_sparse_rrr_type;
typedef runs_vectors::partitioned_zombit_vector_sparse<sdsl::hyb_vector<>> pzombit_sparse_hyb_type;
typedef sdsl::rrr_vector<15> rrr_type;
typedef sdsl::hyb_vector<> hyb_type;
typedef sdsl::sd_vector<> sd_type;
typedef sdsl::sd_vector<>::rank_1_type sd_rank_type;
typedef sdsl::sd_vector<>::select_1_type sd_select_type;
typedef sdsl::succ_support_sd<> sd_succ_type;
typedef runs_vectors::oz_vector<> oz_type;

#define TIMES 5



template<class Bitmap, class Rank, class Select, class Succ>
void run(std::vector<uint64_t> &qs, std::string index_file){


    Bitmap bitmap;
    Rank rank;
    Select select;
    Succ succ;

    std::string rank_file_name = index_file + ".rank";
    std::string succ_file_name = index_file + ".succ";
    std::string select_file_name = index_file + ".select";
    sdsl::load_from_file(bitmap, index_file);
    sdsl::load_from_file(rank, rank_file_name);
    sdsl::load_from_file(succ, succ_file_name);
    rank.set_vector(&bitmap);
    succ.set_vector(&bitmap);
    select.set_vector(&bitmap);

    auto size = sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ) + sdsl::size_in_bytes(select);

    uint64_t sum = 0;
    double avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += bitmap[qs[i]];
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += bitmap[qs[i]];
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Access " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_access = avg / TIMES;
    std::cout << "Access avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += rank(qs[i]+1);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += rank(qs[i]+1);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Rank " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_rank = avg / TIMES;
    std::cout << "Rank avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    uint64_t cnt = rank(qs.size());
    double avg_select;
    if(cnt == 0){
        avg_select = 0;
    }else {
        std::vector<uint64_t> pos_sel;
        for (uint64_t i = 0; i < qs.size(); ++i) {
            pos_sel.push_back(qs[i] % cnt);
            sum += select(pos_sel.back() + 1);
        }
        for (uint64_t t = 0; t < TIMES; ++t) {
            sum = 0;
            auto t1 = std::chrono::high_resolution_clock::now();
            for (uint64_t i = 0; i < pos_sel.size(); ++i) {
                sum += select(pos_sel[i] + 1);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
            avg += time / (double) qs.size();
            std::cout << "Select " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum
                      << "]" << std::endl;
        }
        avg_select = avg / TIMES;
        std::cout << "Select avg per query: " << avg / TIMES << " ns." << std::endl;
        std::cout << std::endl;
        pos_sel.clear();
        pos_sel.shrink_to_fit();
    }


    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += succ(qs[i]);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += succ(qs[i]);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Succ " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    std::cout << "Succ avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;
    auto avg_succ = avg / TIMES;

    std::cerr << size << ";" << avg_access << ";" << avg_rank << ";" << avg_succ << ";" << avg_select << std::endl;
}


template<class Bitmap, class Rank, class Select, class Succ>
void run_zombit(std::vector<uint64_t> &qs, std::string &index_file){

    Bitmap bitmap;
    Rank rank;
    Select select;
    Succ succ;

    std::string rank_file_name = index_file + ".rank";
    sdsl::load_from_file(bitmap, index_file);
    sdsl::load_from_file(rank, rank_file_name);
    rank.set_vector(&bitmap);
    sdsl::util::init_support(succ, &bitmap);
    select.set_rank(&rank);

    auto size = sdsl::size_in_bytes(bitmap) + sdsl::size_in_bytes(rank) + sdsl::size_in_bytes(succ) + sdsl::size_in_bytes(select);

    uint64_t sum = 0;
    double avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += bitmap[qs[i]];
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += bitmap[qs[i]];
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Access " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_access = avg / TIMES;
    std::cout << "Access avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += rank(qs[i]+1);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += rank(qs[i]+1);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Rank " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    auto avg_rank = avg / TIMES;
    std::cout << "Rank avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;

    sum = 0; avg = 0;
    uint64_t cnt = rank(qs.size());
    double avg_select;
    if(cnt == 0){
        avg_select = 0;
    }else{
        std::vector<uint64_t> pos_sel;
        for(uint64_t i = 0; i < qs.size(); ++i){
            pos_sel.push_back(qs[i] % cnt);
            sum += select(pos_sel.back()+1);
        }
        for(uint64_t t = 0; t < TIMES; ++t){
            sum = 0;
            auto t1 = std::chrono::high_resolution_clock::now();
            for(uint64_t i = 0; i < pos_sel.size(); ++i){
                sum += select(pos_sel[i]+1);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
            avg += time / (double) qs.size();
            std::cout << "Select " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
        }
        avg_select = avg / TIMES;
        std::cout << "Select avg per query: " << avg / TIMES << " ns." << std::endl;
        std::cout << std::endl;
        pos_sel.clear();
        pos_sel.shrink_to_fit();
    }



    sum = 0; avg = 0;
    for(uint64_t i = 0; i < qs.size(); ++i){
        sum += succ(qs[i]);
    }
    for(uint64_t t = 0; t < TIMES; ++t){
        sum = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < qs.size(); ++i){
            sum += succ(qs[i]);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        avg += time / (double) qs.size();
        std::cout << "Succ " << time << " ns (per query: " << time / (double) qs.size() << " ns.)[sum=" << sum << "]" << std::endl;
    }
    std::cout << "Succ avg per query: " << avg / TIMES << " ns." << std::endl;
    std::cout << std::endl;
    auto avg_succ = avg / TIMES;

    std::cerr << size << ";" << avg_access << ";" << avg_rank << ";" << avg_succ << ";" << avg_select << std::endl;
}


int main(int argc, char** argv)
{
    std::string file_name = argv[1];
    std::string file_q = argv[2];
    std::vector<uint64_t> q;
    util::file::read_from_file(file_q, q);

    std::cout << "---- HYB ----" << std::endl;
    std::string index_file = file_name + ".hyb";
    run_zombit<hyb_type, typename hyb_type::rank_1_type, typename hyb_type::select_1_type, typename hyb_type::succ_1_type>(q, index_file);

    index_file = file_name + ".rrr";
    std::cout << "---- RRR ----" << std::endl;
    run<rrr_type, typename rrr_type::rank_1_type, typename rrr_type::select_1_type, typename rrr_type::succ_1_type>(q, index_file);

    index_file = file_name + ".oz";
    std::cout << "---- OZ ----" << std::endl;
    run<oz_type, typename oz_type::rank_1_type, typename oz_type::select_1_type, typename oz_type::succ_1_type>(q, index_file);

    index_file = file_name + ".sd";
    std::cout << "---- SD ----" << std::endl;
    run<sd_type, typename sd_type::rank_1_type, typename sd_type::select_1_type, sd_succ_type>(q, index_file);

    std::cout << "---- Zombit ----" << std::endl;
    index_file = file_name + ".zombit";
    run_zombit<zombit_plain_type, typename zombit_plain_type::rank_1_type, typename zombit_plain_type::select_1_type, typename zombit_plain_type::succ_1_type>(q, index_file);

    std::cout << "---- Zombit Naive ----" << std::endl;
    index_file = file_name + ".zombit.naive";
    run_zombit<zombit_plain_type, zombit_plain_rank_type, zombit_plain_select_type , zombit_plain_succ_naive_type> (q, index_file);

    std::cout << "---- Zombit RRR----" << std::endl;
    index_file = file_name + ".zombit.rrr";
    run_zombit<zombit_rrr_type, typename zombit_rrr_type::rank_1_type, typename zombit_rrr_type::select_1_type, typename zombit_rrr_type::succ_1_type>(q, index_file);

    std::cout << "---- Zombit HYB----" << std::endl;
    index_file = file_name + ".zombit.hyb";
    run_zombit<zombit_hyb_type, typename zombit_hyb_type::rank_1_type, typename zombit_hyb_type::select_1_type, typename zombit_hyb_type::succ_1_type>(q, index_file);

    std::cout << "---- Zombit v2 ----" << std::endl;
    index_file = file_name + ".zombit.v2";
    run_zombit<zombit_v2_plain_type, typename zombit_v2_plain_type::rank_1_type, typename zombit_v2_plain_type::select_1_type, typename zombit_v2_plain_type::succ_1_type>(q, index_file);

    std::cout << "---- Zombit v2 Naive ----" << std::endl;
    index_file = file_name + ".zombit.v2.naive";
    run_zombit<zombit_v2_plain_type, zombit_v2_plain_rank_type, zombit_v2_plain_select_type, zombit_v2_plain_succ_naive_type> (q, index_file);

    std::cout << "---- Zombit v2 RRR----" << std::endl;
    index_file = file_name + ".zombit.v2.rrr";
    run_zombit<zombit_v2_rrr_type, typename zombit_v2_rrr_type::rank_1_type, typename zombit_v2_rrr_type::select_1_type, typename zombit_v2_rrr_type::succ_1_type>(q, index_file);

    std::cout << "---- Zombit v2 HYB----" << std::endl;
    index_file = file_name + ".zombit.v2.hyb";
    run_zombit<zombit_v2_hyb_type, typename zombit_v2_hyb_type::rank_1_type, typename zombit_v2_hyb_type::select_1_type, typename zombit_v2_hyb_type::succ_1_type>(q, index_file);

    std::cout << "---- PZombit ----" << std::endl;
    index_file = file_name + ".pzombit";
    run_zombit<pzombit_plain_type, typename pzombit_plain_type::rank_1_type, typename pzombit_plain_type::select_1_type, typename pzombit_plain_type::succ_1_type>(q, index_file);

    std::cout << "---- PZombit Naive ----" << std::endl;
    index_file = file_name + ".pzombit.naive";
    run_zombit<pzombit_plain_type, pzombit_plain_rank_type,  pzombit_plain_select_type, pzombit_plain_succ_naive_type> (q, index_file);

    std::cout << "---- PZombit RRR----" << std::endl;
    index_file = file_name + ".pzombit.rrr";
    run_zombit<pzombit_rrr_type, typename pzombit_rrr_type::rank_1_type, typename pzombit_rrr_type::select_1_type, typename pzombit_rrr_type::succ_1_type>(q, index_file);

    std::cout << "---- PZombit HYB----" << std::endl;
    index_file = file_name + ".pzombit.hyb";
    run_zombit<pzombit_hyb_type, typename pzombit_hyb_type::rank_1_type, typename pzombit_hyb_type::select_1_type, typename pzombit_hyb_type::succ_1_type>(q, index_file);

    std::cout << "---- PZombit v2 ----" << std::endl;
    index_file = file_name + ".pzombit.v2";
    run_zombit<pzombit_sparse_plain_type, typename pzombit_sparse_plain_type::rank_1_type, typename pzombit_sparse_plain_type::select_1_type, typename pzombit_sparse_plain_type::succ_1_type>(q, index_file);

    std::cout << "---- PZombit v2 Naive ----" << std::endl;
    index_file = file_name + ".pzombit.v2.naive";
    run_zombit<pzombit_sparse_plain_type, pzombit_sparse_plain_rank_type, pzombit_sparse_plain_select_type, pzombit_sparse_plain_succ_naive_type> (q, index_file);

    std::cout << "---- PZombit v2 RRR----" << std::endl;
    index_file = file_name + ".pzombit.v2.rrr";
    run_zombit<pzombit_sparse_rrr_type, typename pzombit_sparse_rrr_type::rank_1_type, typename pzombit_sparse_rrr_type::select_1_type, typename pzombit_sparse_rrr_type::succ_1_type>(q, index_file);

    std::cout << "---- PZombit v2 zHYB----" << std::endl;
    index_file = file_name + ".pzombit.v2.hyb";
    run_zombit<pzombit_sparse_hyb_type, typename pzombit_sparse_hyb_type::rank_1_type, typename pzombit_sparse_hyb_type::select_1_type, typename pzombit_sparse_hyb_type::succ_1_type>(q, index_file);



}

