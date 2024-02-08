/***
BSD 2-Clause License

Copyright (c) 2018, Adrián
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/


//
// Created by Adrián on 17/12/2018.
//

#ifndef RUNS_VECTORS_PARTITIONED_ZOMBIT_VECTOR_HPP
#define RUNS_VECTORS_PARTITIONED_ZOMBIT_VECTOR_HPP

#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support.hpp>
#include <algorithm>
#include <cstdint>
#include <sdsl/succ_support_v.hpp>
#include <sdsl/select_support.hpp>
#include <util.hpp>
#include <optimal_byte_partition.hpp>
#include <sdsl/sd_vector.hpp>
#include <sdsl/math.hpp>


#define ZOMBIT_SMALL_BLOCK 8

namespace runs_vectors {

    template<uint8_t t_b, class t_mixed> class rank_support_partitioned_zombit_simple;
    template<uint8_t t_b, class t_mixed> class rank_support_partitioned_zombit_v;
    template<uint8_t t_b, class t_mixed> class rank_support_rec_partitioned_zombit;
    template<uint8_t t_b, class t_mixed> class select_support_zombit_v4;
    template<uint8_t t_b, class t_mixed> class succ_support_partitioned_zombit_naive;
    template<uint8_t t_b, class t_mixed> class succ_support_partitioned_zombit;
    template<class t_mixed> class rec_partitioned_zombit_vector;
    class succ_support_rec_partitioned_zombit;


    template <class t_mixed = sdsl::bit_vector>
    class partitioned_zombit_vector {

    public:
        typedef sdsl::bit_vector    bitmap_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_bitmap_type;
        typedef sdsl::sd_vector<>   length_type;
        typedef typename sdsl::sd_vector<>::rank_1_type   rank_length_type;
        typedef typename sdsl::sd_vector<>::select_1_type select_length_type;
        typedef t_mixed  mixed_bitmap_type;
        typedef typename sdsl::bit_vector::size_type size_type;
        typedef typename sdsl::bit_vector::value_type value_type;
        typedef typename sdsl::bit_vector::difference_type difference_type;
        typedef sdsl::random_access_const_iterator<partitioned_zombit_vector> iterator;
        typedef rank_support_partitioned_zombit_v<1, t_mixed> rank_1_type;
        typedef rank_support_partitioned_zombit_v<0, t_mixed> rank_0_type;
        typedef select_support_zombit_v4<1, t_mixed> select_1_type;
        typedef select_support_zombit_v4<0, t_mixed> select_0_type;
        typedef succ_support_partitioned_zombit<1, t_mixed> succ_1_type;

        friend class select_support_zombit_v4<1, t_mixed>;
        friend class select_support_zombit_v4<0, t_mixed>;
        friend class rank_support_partitioned_zombit_simple<1, t_mixed>;
        friend class rank_support_partitioned_zombit_simple<0, t_mixed>;
        friend class rank_support_partitioned_zombit_v<1, t_mixed>;
        friend class rank_support_rec_partitioned_zombit<0, t_mixed>;
        friend class succ_support_partitioned_zombit_naive<1, t_mixed>;
        friend class succ_support_partitioned_zombit<1, t_mixed>;
        friend class rec_partitioned_zombit_vector<mixed_bitmap_type>;
        friend class rank_support_rec_partitioned_zombit<1, t_mixed>;
        friend class rank_support_partitioned_zombit_v<0, t_mixed>;
        friend class succ_support_rec_partitioned_zombit;

    private:

        size_type m_size = 0;
        length_type m_partitions;
        rank_length_type m_rank_partitions;
        select_length_type m_select_partitions;
        bitmap_type m_full;
        rank_bitmap_type m_rank_full;
        bitmap_type m_info;
        length_type m_length_mixed;
        select_length_type m_select_length_mixed;
        mixed_bitmap_type m_mixed;

        void copy(const partitioned_zombit_vector &o){
            m_size = o.m_size;
            m_partitions = o.m_partitions;
            m_rank_partitions = o.m_rank_partitions;
            m_select_partitions = o.m_select_partitions;
            m_rank_partitions.set_vector(&m_partitions);
            m_select_partitions.set_vector(&m_partitions);
            m_full = o.m_full;
            m_rank_full = o.m_rank_full;
            m_rank_full.set_vector(&m_full);
            m_info = o.m_info;
            m_length_mixed = o.m_length_mixed;
            m_select_length_mixed = o.m_select_length_mixed;
            m_select_length_mixed.set_vector(&m_length_mixed);
            m_mixed = o.m_mixed;
        }

        void get_small_blocks(const sdsl::bit_vector &c, std::vector<partition::block_t> &blocks_vec){
            const uint8_t* data = (uint8_t*) c.data();
            uint64_t blocks = sdsl::util::math::ceil_div(c.size(), ZOMBIT_SMALL_BLOCK);
            blocks_vec.resize(blocks);
            for(uint64_t i = 0; i < blocks; ++i){
                if(i < blocks-1){
                    if(data[i] == 0){
                        blocks_vec[i] = partition::block_t::run0;
                    }else if (data[i] == 255){
                        blocks_vec[i] = partition::block_t::run1;
                    }else{
                        blocks_vec[i] = partition::block_t::mixed;
                    }
                }else{
                    uint8_t mask = sdsl::bits::lo_set[c.size()-i*ZOMBIT_SMALL_BLOCK];
                    uint8_t aux = data[i] & mask;
                    if(aux == 0){
                        blocks_vec[i] = partition::block_t::run0;
                    }else if (aux == mask){
                        blocks_vec[i] = partition::block_t::run1;
                    }else{
                        blocks_vec[i] = partition::block_t::mixed;
                    }
                }
            }
        }

        inline size_type pos_to_block(const size_type pos) const{
            return m_rank_partitions(pos/ZOMBIT_SMALL_BLOCK+1) - 1;
        }

        inline size_type begin_block(const size_type b) const{
            return m_select_partitions(b+1) * ZOMBIT_SMALL_BLOCK;
        }

        inline size_type begin_mixed_block(const size_type mb) const{
            return m_select_length_mixed(mb+1) * ZOMBIT_SMALL_BLOCK;
        }


    public:

        const bitmap_type &full = m_full;
        const bitmap_type &info = m_info;
        const mixed_bitmap_type &mixed = m_mixed;
        const select_length_type &select_partitions = m_select_partitions;
        const select_length_type &select_length_mixed = m_select_length_mixed;

        partitioned_zombit_vector() = default;



        explicit partitioned_zombit_vector(const sdsl::bit_vector &c){

            if(c.empty()) return;

            //Computing runs and sample
            m_size = c.size();
            std::vector<partition::block_t> small_blocks;
            get_small_blocks(c, small_blocks);
            partition::optimal_variable opt_var(small_blocks.begin(), small_blocks.size(), c.size());
            size_type n_blocks = opt_var.partition.size();

            //Computing full and mixed blocks
            m_full = sdsl::bit_vector(n_blocks, 0);
            m_info = sdsl::bit_vector(n_blocks, 0);

            //Computing full and mixed blocks
            size_type i_sb = 0, cnt_mixed_blocks = 0;
            std::vector<std::pair<size_type, size_type>> mixed_blocks;
           // std::vector<bool> is_mixed(n_blocks, false);
            for(size_type i_p = 0; i_p < n_blocks; ++i_p) {
                size_type cnt_run0 = 0, cnt_run1 = 0, cnt_mixed = 0;
                size_type beg_sb = i_sb;
                while (i_sb < opt_var.partition[i_p]) {
                    if (small_blocks[i_sb] == partition::run0) {
                        ++cnt_run0;
                    } else if (small_blocks[i_sb] == partition::run1) {
                        ++cnt_run1;
                    } else {
                        ++cnt_mixed;
                       // is_mixed[i_sb] = true;
                    }
                    ++i_sb;
                }

                if (cnt_mixed || (cnt_run0 && cnt_run1)) { //Mixed
                    m_full[i_p] = 0;
                    m_info[i_p] = 1;
                    cnt_mixed_blocks += opt_var.partition[i_p] - beg_sb;
                    mixed_blocks.emplace_back(beg_sb, opt_var.partition[i_p]);
                } else if (cnt_run1) { //Full Ones
                    m_full[i_p] = 1;
                    m_info[i_p] = 1;
                } else { //Empty
                    m_full[i_p] = 1;
                    m_info[i_p] = 0;
                }
            }
            //tricks for avoiding if statements in succ
            m_info.resize(m_info.size()+1);
            m_info[m_info.size()-1] = 1;
            m_full.resize(m_full.size()+2);
            m_full[m_full.size()-2] = 0;
            m_full[m_full.size()-1] = 1;
            //Storing info of mixed blocks
            auto aux = sdsl::bit_vector(cnt_mixed_blocks*ZOMBIT_SMALL_BLOCK);
            size_type ith_mixed = 0, start_block, end_block;
            for(const auto &mixed_block : mixed_blocks){
                start_block = mixed_block.first * ZOMBIT_SMALL_BLOCK;
                end_block = std::min(mixed_block.second * ZOMBIT_SMALL_BLOCK, c.size());
                for(size_type i = start_block; i < end_block; ++i){
                    aux[ith_mixed] = c[i];
                    ++ith_mixed;
                }
            }
            m_mixed = mixed_bitmap_type(aux);
            //Storing lengths
            {
                sdsl::bit_vector part_aux(small_blocks.size()+1, 0);
                part_aux[0]=1;
                for(size_type ith_block = 0; ith_block < n_blocks; ++ith_block){
                    part_aux[opt_var.partition[ith_block]] = 1;
                }
                //part_aux[n_blocks]=1;
                m_partitions = sdsl::sd_vector<>(part_aux);
            }


            {
                sdsl::bit_vector mix_aux(cnt_mixed_blocks+1, 0);
                mix_aux[0]=1;
                size_type last = 0;
                for(const auto &mixed_block : mixed_blocks){
                    auto len = mixed_block.second - mixed_block.first;
                    last = last + len;
                    mix_aux[last] = 1;
                }
                mix_aux[cnt_mixed_blocks]=1;
                m_length_mixed = sdsl::sd_vector<>(mix_aux);
            }

            sdsl::util::init_support(m_rank_full, &m_full);
            sdsl::util::init_support(m_rank_partitions, &m_partitions);
            sdsl::util::init_support(m_select_partitions, &m_partitions);
            sdsl::util::init_support(m_select_length_mixed, &m_length_mixed);
        }

        //! Copy constructor
        partitioned_zombit_vector(const partitioned_zombit_vector& o)
        {
            copy(o);
        }

        //! Move constructor
        partitioned_zombit_vector(partitioned_zombit_vector&& o)
        {
            *this = std::move(o);
        }


        partitioned_zombit_vector &operator=(const partitioned_zombit_vector &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }
        partitioned_zombit_vector &operator=(partitioned_zombit_vector &&o) {
            if (this != &o) {
                m_size = o.m_size;
                m_partitions = std::move(o.m_partitions);
                m_rank_partitions = std::move(o.m_rank_partitions);
                m_rank_partitions.set_vector(&m_partitions);
                m_select_partitions = std::move(o.m_select_partitions);
                m_select_partitions.set_vector(&m_partitions);
                m_full = std::move(o.m_full);
                m_rank_full = std::move(o.m_rank_full);
                m_rank_full.set_vector(&m_full);
                m_info = std::move(o.m_info);
                m_length_mixed = std::move(o.m_length_mixed);
                m_select_length_mixed = std::move(o.m_select_length_mixed);
                m_select_length_mixed.set_vector(&m_length_mixed);
                m_mixed = std::move(o.m_mixed);
            }
            return *this;
        }

        void swap(partitioned_zombit_vector &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_size, o.m_size);
            m_partitions.swap( o.m_partitions);
            sdsl::util::swap_support(m_rank_partitions, o.m_rank_partitions,
                                     &m_partitions, &o.m_partitions);
            sdsl::util::swap_support(m_select_partitions, o.m_select_partitions,
                                     &m_partitions, &o.m_partitions);
            m_full.swap(o.m_full);
            sdsl::util::swap_support(m_rank_full, o.m_rank_full, &m_full, &o.m_full);
            m_info.swap(o.m_info);
            m_length_mixed.swap(o.m_length_mixed);
            sdsl::util::swap_support(m_select_length_mixed, o.m_select_length_mixed, &m_length_mixed, &o.m_length_mixed);
            m_mixed.swap(o.m_mixed);
        }

        inline value_type access(const size_type i) const{

            //std::cout << "acess at " << i << std::endl;
            auto j = pos_to_block(i);
            if(m_full[j]) return m_info[j];
            auto mix_b = j - m_rank_full(j+1);
            auto delta = i - begin_block(j);
            return m_mixed[begin_mixed_block(mix_b) + delta];

        }

        inline value_type operator[](const size_type i) const{
            return access(i);
        }

        inline size_type size() const {
            return m_size;
        }

        iterator begin() const
        {
            return iterator(this, 0);
        }

        iterator end() const
        {
            return iterator(this, size());
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_partitions.serialize(out, child, "partitions");
            written_bytes += m_rank_partitions.serialize(out, child, "rank_partitions");
            written_bytes += m_select_partitions.serialize(out, child, "select_partitions");
            written_bytes += m_full.serialize(out, child, "full");
            written_bytes += m_rank_full.serialize(out, child, "rank_full");
            written_bytes += m_info.serialize(out, child, "info");
            written_bytes += m_length_mixed.serialize(out, child, "length_mixed");
            written_bytes += m_select_length_mixed.serialize(out, child, "select_length_mixed");
            written_bytes += m_mixed.serialize(out, child, "mixed");
            written_bytes += sdsl::serialize(m_size, out, child, "size");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream& in)
        {
            m_partitions.load(in);
            m_rank_partitions.load(in, &m_partitions);
            m_select_partitions.load(in, &m_partitions);
            m_full.load(in);
            m_rank_full.load(in, &m_full);
            m_info.load(in);
            m_length_mixed.load(in);
            m_select_length_mixed.load(in, &m_length_mixed);
            m_mixed.load(in);
            sdsl::load(m_size, in);
        }

        /*void print(){
            std::cout << "sample=" << sample << std::endl;
            std::cout << "full={";
            for(auto i = 0; i < m_full.size(); ++i){
                std::cout << m_full[i] << ", ";
            }
            std::cout << "}" << std::endl;

            std::cout << "info={";
            for(auto i = 0; i < m_info.size(); ++i){
                std::cout << m_info[i] << ", ";
            }
            std::cout << "}" << std::endl;

            std::cout << "mixed={";
            for(auto i = 0; i < m_mixed.size(); ++i){
                std::cout << m_mixed[i] << ", ";
            }
            std::cout << "}" << std::endl;
        }*/

    };



    template<uint8_t t_b>
    struct rank_support_partitioned_zombit_trait {
        typedef partitioned_zombit_vector<>::size_type size_type;
        static size_type adjust_rank(size_type r,size_type)
        {
            return r;
        }
    };

    template<>
    struct rank_support_partitioned_zombit_trait<0> {
        typedef partitioned_zombit_vector<>::size_type size_type;
        static size_type adjust_rank(size_type r, size_type n)
        {
            return n - r;
        }
    };

    template<uint8_t t_b, class t_mixed>
    class rank_support_partitioned_zombit_simple {

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename t_mixed::rank_1_type rank_mixed_type;
    private:
        const partitioned_zombit_vector<t_mixed>* m_v = nullptr;

        rank_mixed_type m_rank_mixed;
        std::vector<uint64_t> m_blocks;

        void copy(const rank_support_partitioned_zombit_simple& ss){
            m_v = ss.m_v;
            m_rank_mixed = ss.m_rank_mixed;
            m_blocks = ss.m_blocks;
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
            }
        }
    public:


        rank_support_partitioned_zombit_simple() = default;

        //! Copy constructor
        rank_support_partitioned_zombit_simple(const rank_support_partitioned_zombit_simple& hybrid)
        {
            copy(hybrid);
        }

        explicit rank_support_partitioned_zombit_simple(const partitioned_zombit_vector<t_mixed>* v)
        {
            m_v = v;
            sdsl::util::init_support(m_rank_mixed, &(m_v->mixed));
            size_type z = 0, o = 0, m = 0, i_zom = 0, i_m = 0;
            size_type cnt = 0;
            size_type n_partitions = m_v->info.size()-1;
            m_blocks.resize(n_partitions);
            for(size_type i = 0; i < n_partitions; ++i){
                m_blocks[i] = cnt;
                if(m_v->full[i]){
                    if(m_v->info[i]){
                        auto next = m_v->select_partitions(i+1);
                        ++o;
                        cnt += next - i_zom;
                        i_zom = next;
                    }else{
                        i_zom = m_v->select_partitions(i+1);
                        ++z;
                    }
                }else{
                    auto next = m_v->select_length_mixed(m+1);
                    cnt += m_rank_mixed(next) - m_rank_mixed(i_m);
                    i_m = next;
                    ++m;
                }
            }

        }


        //! Returns the position of the i-th occurrence in the bit vector.
        size_type rank(size_type i)const
        {
            assert(i >= 0); assert(i <= m_v->size());
            if(i <= 0) return 0;
            auto j = m_v->pos_to_block(i-1); //block
            size_type r;
            if(m_v->full[j]){
                if(m_v->m_info[j]){
                    r = m_blocks[j] + (i - m_v->begin_block(j));
                }else{
                    r =  m_blocks[j];
                }
            }else{
                size_type m = j - m_v->m_rank_full(j+1)+1;
                size_type delta = i - m_v->begin_block(j);
                size_type beg = m_v->begin_mixed_block(m);
                r = r + m_rank_mixed(beg + delta) - m_rank_mixed(beg);
            }
            return rank_support_partitioned_zombit_trait<t_b>::adjust_rank(r, i);
        }


        size_type operator()(size_type i)const
        {
            return rank(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                sdsl::util::init_support(m_rank_mixed, &(m_v->mixed));
            }

        }

        rank_support_partitioned_zombit_simple& operator=(const rank_support_partitioned_zombit_simple& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        rank_support_partitioned_zombit_simple& operator=(rank_support_partitioned_zombit_simple& ss)
        {

            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_rank_mixed = std::move(ss.m_rank_mixed);
                m_blocks = std::move(ss.m_blocks);
                if(m_v != nullptr){
                    m_rank_mixed.set_vector(&(m_v->mixed));
                }
            }
            return *this;
        }

        void swap(rank_support_partitioned_zombit_simple& o) {
            std::swap(m_v, o.m_v);
            m_rank_mixed.swap(o.m_rank_mixed);
            std::swap(m_blocks, o.m_blocks);
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
            }else{
                m_rank_mixed.set_vector(nullptr);
            }
            if(o.m_v != nullptr){
                o.m_rank_mixed.set_vector(&(o.m_v->mixed));
            }else{
                o.m_rank_mixed.set_vector(nullptr);
            }


        }



        void load(std::istream& in, const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                m_rank_mixed.load(in, &(m_v->mixed));
            }
            m_blocks.resize(m_v->info.size()-1);
            sdsl::load_vector(m_blocks, in);
        }

        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_rank_mixed.serialize(out, child, "rank_mixed");
            written_bytes += sdsl::serialize_vector(m_blocks, out, child, "blocks");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };


    template<uint8_t t_b, class t_mixed>
    class rank_support_partitioned_zombit_v {

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename t_mixed::rank_1_type rank_mixed_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_type;
        typedef sdsl::rank_support_trait<t_b, 1> trait_type;
        friend class rank_support_rec_partitioned_zombit<t_b, t_mixed>;
    private:
        const partitioned_zombit_vector<t_mixed>* m_v = nullptr;

        rank_mixed_type m_rank_mixed;
        rank_type m_rank_info;
        sdsl::sd_vector<>  m_length_o;
        sdsl::select_support_sd<1> m_select_length_o;

        void copy(const rank_support_partitioned_zombit_v& ss){
            m_v = ss.m_v;
            m_rank_mixed = ss.m_rank_mixed;
            m_rank_info = ss.m_rank_info;
            m_length_o = ss.m_length_o;
            m_select_length_o.set_vector(&m_length_o);
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
                m_rank_info.set_vector(&(m_v->info));
            }
        }
    public:


        rank_support_partitioned_zombit_v() = default;

        //! Copy constructor
        rank_support_partitioned_zombit_v(const rank_support_partitioned_zombit_v& hybrid)
        {
            copy(hybrid);
        }

        explicit rank_support_partitioned_zombit_v(const partitioned_zombit_vector<t_mixed>* v)
        {
            m_v = v;
            sdsl::util::init_support(m_rank_mixed, &(m_v->mixed));
            sdsl::util::init_support(m_rank_info, &(m_v->info));
            size_type n_partitions = m_v->info.size()-1;
            sdsl::bit_vector aux(m_v->m_partitions.size(), 0);
            aux[0]=1;
            size_type last = 0;
            for(size_type i = 1; i <= n_partitions; ++i){
                if(m_v->info[i-1] && m_v->full[i-1]){
                    if(i>1){
                        last = last + (m_v->m_select_partitions(i+1) - m_v->m_select_partitions(i));
                    }else{
                        last = m_v->m_select_partitions(i+1);
                    }
                    aux[last] = 1;
                }
            }
            aux.resize(last+1);
            m_length_o = sdsl::sd_vector<>(aux);
            sdsl::util::init_support(m_select_length_o, &m_length_o);
        }


        //! Returns the position of the i-th occurrence in the bit vector.
        size_type rank(size_type i)const
        {
            assert(i >= 0); assert(i <= m_v->size());
            if(i <= 0) return 0;
            auto j = m_v->pos_to_block(i-1); //block

            size_type m = j - m_v->m_rank_full(j); //mixed blocks before j
            size_type o = m_rank_info(j) - m; //ones blocks before j
            size_type r = o ? m_select_length_o(o+1)*ZOMBIT_SMALL_BLOCK : 0; //1-bits of ones blocks
            if(m_v->full[j]){
                r += m ? m_rank_mixed(m_v->m_select_length_mixed(m+1)*ZOMBIT_SMALL_BLOCK) : 0; //1-bits of mixed blocks
                if(m_v->info[j]) {
                    r += i - m_v->begin_block(j); //1-bits of current block
                }
            }else{
                r += m_rank_mixed(m_v->m_select_length_mixed(m+1)*ZOMBIT_SMALL_BLOCK + (i - m_v->begin_block(j))); //1-bits of mixed blocks including the current one
            }
            return rank_support_partitioned_zombit_trait<t_b>::adjust_rank(r, i);
        }


        size_type operator()(size_type i)const
        {
            return rank(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                sdsl::util::init_support(m_rank_mixed, &(m_v->mixed));
            }

        }

        rank_support_partitioned_zombit_v& operator=(const rank_support_partitioned_zombit_v& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        rank_support_partitioned_zombit_v& operator=(rank_support_partitioned_zombit_v& ss)
        {

            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_rank_mixed = std::move(ss.m_rank_mixed);
                m_rank_info = std::move(ss.m_rank_info);
                m_length_o = std::move(ss.m_length_o);
                m_select_length_o.set_vector(&m_length_o);
                if(m_v != nullptr){
                    m_rank_mixed.set_vector(&(m_v->mixed));
                    m_rank_info.set_vector(&(m_v->info));
                }
            }
            return *this;
        }

        void swap(rank_support_partitioned_zombit_v& o) {
            std::swap(m_v, o.m_v);
            m_rank_mixed.swap(o.m_rank_mixed);
            m_rank_info.swap(o.m_rank_info);
            m_length_o.swap(o.m_length_o);
            sdsl::util::swap_support(m_select_length_o, o.m_select_length_o, &m_length_o, &o.m_length_o);
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
                m_rank_info.set_vector(&(m_v->info));
            }else{
                m_rank_mixed.set_vector(nullptr);
                m_rank_info.set_vector(nullptr);
            }
            if(o.m_v != nullptr){
                o.m_rank_mixed.set_vector(&(o.m_v->mixed));
                o.m_rank_info.set_vector(&(o.m_v->info));
            }else{
                o.m_rank_mixed.set_vector(nullptr);
                o.m_rank_info.set_vector(nullptr);
            }


        }



        void load(std::istream& in, const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                m_rank_mixed.load(in, &(m_v->mixed));
                m_rank_info.load(in, &(m_v->info));
            }
            m_length_o.load(in);
            m_select_length_o.load(in, &m_length_o);
        }

        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_rank_mixed.serialize(out, child, "rank_mixed");
            written_bytes += m_rank_info.serialize(out, child, "rank_info");
            written_bytes += m_length_o.serialize(out, child, "length_o");
            written_bytes += m_select_length_o.serialize(out, child, "select_length_o");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };


    template<uint8_t t_b, class t_mixed = sdsl::bit_vector>
    class succ_support_partitioned_zombit {

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename t_mixed::succ_1_type succ_1_mixed_type;
    private:
        const partitioned_zombit_vector<t_mixed>* m_v;
        sdsl::succ_support_v<1>                   m_succ_info;
        succ_1_mixed_type                         m_succ_mixed;


        void copy(const succ_support_partitioned_zombit& ss){
            m_v = ss.m_v;
            m_succ_info = ss.m_succ_info;
            m_succ_mixed = ss.m_succ_mixed;
            if(m_v != nullptr){
                m_succ_info.set_vector(&m_v->m_info);
                m_succ_mixed.set_vector(&(m_v->m_mixed));
            }else{
                m_succ_info.set_vector(nullptr);
                m_succ_mixed.set_vector(nullptr);
            }
        }

        inline size_type next_block(size_type j) const {
            j = m_succ_info( j+1); //Next partition with 1-bits
            if(j >= m_v->m_info.size()-1) return m_v->size();
            if(m_v->m_full[j]){
                return m_v->begin_block(j); //Full of ones
            }else{
                size_type m = j-m_v->m_rank_full(j+1);
                size_type lb = m_v->begin_mixed_block(m);
                auto next_m = m_succ_mixed(lb);
                if(next_m == m_v->mixed.size()) return m_v->size();
                return (next_m - lb) + m_v->begin_block(j);
            }
        }

        inline size_type next_block(size_type j, size_type next_m, size_type lb) const {
            j = m_succ_info( j+1);
            if(j >= m_v->m_info.size()-1) return m_v->size();
            if(m_v->m_full[j]){
                return m_v->begin_block(j);
            }else{
                if(next_m == m_v->mixed.size()) return m_v->size();
                return (next_m - lb) + m_v->begin_block(j);
            }
        }

    public:

        //! Copy constructor
        succ_support_partitioned_zombit(const succ_support_partitioned_zombit& hybrid)
        {
            copy(hybrid);
        }

        succ_support_partitioned_zombit(const partitioned_zombit_vector<t_mixed>* v = nullptr)
        {
            m_v = v;
            if(m_v == nullptr) return;
            sdsl::util::init_support(m_succ_info, &(m_v->m_info));
            sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
        }



        inline size_type succ(size_type i) const{

            auto j = m_v->pos_to_block(i);
            if(m_v->m_full[j]){
                if (m_v->m_info[j] == 1) {
                    return i;
                }else{
                    return next_block(j);
                }
            }else{
                size_type m = j-m_v->m_rank_full(j+1);
                size_type beg = m_v->begin_block(j);
                size_type delta = (i - beg);
                size_type lb = m_v->begin_mixed_block(m);
                size_type rb = m_v->begin_mixed_block(m+1);
                auto next_m = m_succ_mixed(lb + delta);
                if (next_m < rb) {
                    return (next_m - lb) + beg;
                }
                return next_block(j, next_m, rb);
            }
        };

        size_type operator()(size_type i)const
        {
            return succ(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                sdsl::util::init_support(m_succ_info, &(m_v->m_info));
                sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
            }
        }

        succ_support_partitioned_zombit& operator=(const succ_support_partitioned_zombit& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_partitioned_zombit& operator=(succ_support_partitioned_zombit&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_succ_info = std::move(ss.m_succ_info);
                m_succ_mixed = std::move(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_succ_info.set_vector(&(m_v->m_info));
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_info.set_vector(nullptr);
                    m_succ_mixed.set_vector(nullptr);
                }
            }
            return *this;
        }

        void swap(succ_support_partitioned_zombit& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
                m_succ_info.swap(ss.m_succ_info);
                m_succ_mixed.swap(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_succ_info.set_vector(&(m_v->m_info));
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_info.set_vector(nullptr);
                    m_succ_mixed.set_vector(nullptr);
                }
                if(ss.m_v != nullptr){
                    ss.m_succ_info.set_vector(&(ss.m_v->m_info));
                    ss.m_succ_mixed.set_vector(&(ss.m_v->m_mixed));
                }else{
                    ss.m_succ_info.set_vector(nullptr);
                    ss.m_succ_mixed.set_vector(nullptr);
                }
            }
        }

        void load(std::istream& in, const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                m_succ_info.load(in, &(m_v->m_info));
                m_succ_mixed.load(in, &(m_v->m_mixed));
            }
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_succ_info.serialize(out, child, "succ_info");
            written_bytes += m_succ_mixed.serialize(out, child, "succ_mixed");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };



    //Only with t_b=1
    template<uint8_t t_b, class t_mixed = sdsl::bit_vector>
    class succ_support_partitioned_zombit_naive {

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
    private:
        const partitioned_zombit_vector<t_mixed>* m_v;


        void copy(const succ_support_partitioned_zombit_naive& ss){
            m_v = ss.m_v;
        }

        inline size_type next_block(size_type j) const {
            const uint64_t* info_data = m_v->m_info.data();
            j = sdsl::bits_more::next_limit(info_data, j+1, m_v->m_info.size());
            if(j >= m_v->m_info.size()-1) return m_v->size();
            if(m_v->m_full[j]){
                return m_v->begin_block(j); //Full of ones
            }else{
                size_type m = j-m_v->m_rank_full(j+1);
                size_type lb = m_v->begin_mixed_block(m);
                const uint64_t* mixed_data = m_v->mixed.data();
                auto next_m = sdsl::bits_more::next_limit(mixed_data, lb, m_v->mixed.size());
                if(next_m == m_v->mixed.size()) return m_v->size();
                return (next_m - lb) + m_v->begin_block(j);
            }
        }

        inline size_type next_block(size_type j, size_type next_m, size_type lb) const {
            const uint64_t* info_data = m_v->m_info.data();
            j = sdsl::bits_more::next_limit(info_data, j+1, m_v->m_info.size());
            if(j >= m_v->m_info.size()-1) return m_v->size();
            if(m_v->m_full[j]){
                return m_v->begin_block(j);
            }else{
                if(next_m == m_v->mixed.size()) return m_v->size();
                return (next_m - lb) + m_v->begin_block(j);
            }
        }

    public:

        //! Copy constructor
        succ_support_partitioned_zombit_naive(const succ_support_partitioned_zombit_naive& hybrid)
        {
            copy(hybrid);
        }

        succ_support_partitioned_zombit_naive(const partitioned_zombit_vector<t_mixed>* v = nullptr)
        {
            m_v = v;
            if(m_v == nullptr) return;
        }

        inline size_type succ(size_type i) const{

            auto j = m_v->pos_to_block(i);
            if(m_v->m_full[j]){
                if (m_v->m_info[j] == 1) {
                    return i;
                }else{
                    return next_block(j);
                }
            }else {
                size_type m = j - m_v->m_rank_full(j + 1);
                size_type beg = m_v->begin_block(j);
                size_type delta = (i - beg);
                size_type lb = m_v->begin_mixed_block(m);
                size_type rb = m_v->begin_mixed_block(m + 1);
                const uint64_t *mixed_data = m_v->mixed.data();
                auto next_m = sdsl::bits_more::next_limit(mixed_data, lb + delta, m_v->mixed.size());
                if (next_m < rb) {
                    return (next_m - lb) + beg;
                }
                return next_block(j, next_m, rb);
            }
        };

        size_type operator()(size_type i)const
        {
            return succ(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
        }

        succ_support_partitioned_zombit_naive& operator=(const succ_support_partitioned_zombit_naive& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_partitioned_zombit_naive& operator=(succ_support_partitioned_zombit_naive&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
            }
            return *this;
        }

        void swap(succ_support_partitioned_zombit_naive& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
            }
        }

        void load(std::istream& in, const partitioned_zombit_vector<t_mixed>* v=nullptr)
        {
            m_v = v;
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            sdsl::structure_tree::add_size(child, 0);
            return 0;
        }


    };

    //! Select support for the hyb_vector class
/*!
 * \tparam t_b            The bit pattern of size one. (so `0` or `1`)
 * \tparam k_sblock_rate  Superblock rate (number of blocks inside superblock)
 * TODO: implement select queries, currently this is dummy class.
 */
    template<uint8_t t_b, class t_mixed>
    class select_support_zombit_v4
    {
    public:
        typedef partitioned_zombit_vector<t_mixed> bit_vector_type;
        typedef typename bit_vector_type::size_type size_type;
        enum { bit_pat = t_b };
        enum { bit_pat_len = (uint8_t)1 };
    private:
        const bit_vector_type* m_v;

    public:
        //! Standard constructor
        explicit select_support_zombit_v4(const bit_vector_type* v = nullptr)
        {
            set_vector(v);
        }

        //! Answers select queries
        size_type select(size_type) const
        {
            fprintf(stderr, "\nzombit_vector: select queries are not currently supported\n");
            std::exit(EXIT_FAILURE);
        }

        //! Shorthand for select(i)
        const size_type operator()(size_type i) const
        {
            return select(i);
        }

        //! Return the size of the original vector
        const size_type size() const
        {
            return m_v->size();
        }

        //! Set the supported vector
        void set_vector(const bit_vector_type* v = nullptr)
        {
            m_v = v;
        }

        //! Assignment operator
        select_support_zombit_v4& operator=(const select_support_zombit_v4& rs)
        {
            if (this != &rs) {
                set_vector(rs.m_v);
            }
            return *this;
        }

        //! Swap method
        void swap(select_support_zombit_v4&) {}

        //! Load the data structure from a stream and set the supported vector
        void load(std::istream&, const bit_vector_type* v = nullptr)
        {
            set_vector(v);
        }

        //! Serializes the data structure into a stream
        size_type serialize(std::ostream&, sdsl::structure_tree_node* v = nullptr, std::string name = "") const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            sdsl::structure_tree::add_size(child, 0);
            return 0;
        }
    };

}

#endif //RCT_ZOMBIT_VECTOR_HPP
