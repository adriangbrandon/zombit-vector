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

#ifndef RUNS_VECTORS_ZOMBIT_VECTOR_V3_HPP
#define RUNS_VECTORS_ZOMBIT_VECTOR_V3_HPP

#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support.hpp>
#include <algorithm>
#include <cstdint>
#include <sdsl/succ_support_v.hpp>
#include <sdsl/select_support.hpp>
#include <util.hpp>

namespace runs_vectors {

    template<uint8_t t_b, class t_mixed> class rank_support_zombit_v3;
    template<uint8_t t_b, class t_mixed> class select_support_zombit_v3;
    template<uint8_t t_b, class t_mixed> class succ_support_zombit_v3;
    class succ_support_zombit_v3_naive;
    class rec_zombit_vector_v3;
    template<uint8_t t_b> class rank_support_zombit_rec_v3;
    template<uint8_t t_b> class succ_support_zombit_rec_v3;


    template <class t_mixed = sdsl::bit_vector>
    class zombit_vector_v3 {

    public:
        typedef sdsl::bit_vector bitmap_type;
        typedef t_mixed  mixed_bitmap_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_bitmap_type;
        typedef typename sdsl::bit_vector::size_type size_type;
        typedef typename sdsl::bit_vector::value_type value_type;
        typedef typename sdsl::bit_vector::difference_type difference_type;
        typedef sdsl::random_access_const_iterator<zombit_vector_v3> iterator;
        typedef rank_support_zombit_v3<1, t_mixed> rank_1_type;
        typedef rank_support_zombit_v3<0, t_mixed> rank_0_type;
        typedef select_support_zombit_v3<1, t_mixed> select_1_type;
        typedef select_support_zombit_v3<0, t_mixed> select_0_type;
        typedef succ_support_zombit_v3<1, t_mixed> succ_1_type;

        friend class rec_zombit_vector_v3;
        friend class rank_support_zombit_rec_v3<1>;
        friend class rank_support_zombit_rec_v3<0>;
        friend class succ_support_zombit_rec_v3<1>;
        friend class succ_support_zombit_rec_v3<0>;
        friend class rank_support_zombit_v3<1, t_mixed>;
        friend class select_support_zombit_v3<1, t_mixed>;
        friend class select_support_zombit_v3<0, t_mixed>;
        friend class rank_support_zombit_v3<0, t_mixed>;
        friend class succ_support_zombit_v3_naive;
        friend class succ_support_zombit_v3<1, t_mixed >;

    private:

        size_type m_size = 0;
        size_type m_sample;
        bitmap_type m_full;
        rank_bitmap_type m_rank_full;
        bitmap_type m_info;
        mixed_bitmap_type m_mixed;

        void copy(const zombit_vector_v3 &o){
            m_size = o.m_size;
            m_sample = o.m_sample;
            m_full = o.m_full;
            m_rank_full = o.m_rank_full;
            m_rank_full.set_vector(&m_full);
            m_info = o.m_info;
            m_mixed = o.m_mixed;
        }

        void compute_runs(const sdsl::bit_vector &c, std::vector<size_type> &runs){
            size_type i = 0;
            runs.push_back(i);
            bool ones = static_cast<bool>(c[0]);
            while(i < c.size()){
                if(c[i] && !ones){ //first element of a run with 1s
                    ones = true;
                    runs.push_back(i);
                }else if (!c[i] && ones){ // first element of a run with 0s
                    ones = false;
                    runs.push_back(i);
                }
                ++i;
            }
            runs.push_back(i);
        }



    public:

        const size_type &sample = m_sample;
        const mixed_bitmap_type &mixed = m_mixed;

        zombit_vector_v3() = default;


        explicit zombit_vector_v3(const sdsl::bit_vector &c, const size_type default_s = 0){

            if(c.empty()) return;

            //Computing runs and sample
            m_size = c.size();
            std::vector<size_type > runs;
            compute_runs(c, runs);
            auto s = 0;
            if(s == 0){
                s = static_cast<size_type >(std::ceil(std::sqrt(m_size/ (double_t) runs.size())));
            }

            //std::cout << "s: " << s << std::endl;
            m_sample = (s + 7) & (-8);
            //std::cout << "sample: " << m_sample << std::endl;

            //Computing full and mixed blocks
            auto n_blocks = static_cast<size_type >(std::ceil(m_size/ (double_t) sample));
            m_full = sdsl::bit_vector(n_blocks, 0);
            m_info = sdsl::bit_vector(n_blocks, 0);


            //Computing full and mixed blocks
            bool is_run_ones = (c[0] == 0);
            size_type start_block = 0, end_block = sample;
            size_type ith_run = 0;
            std::vector<size_type> mixed_blocks;
            for(size_type ith_block = 0; ith_block <n_blocks; ++ith_block){
                if(runs[ith_run] > start_block && runs[ith_run] < end_block){
                    // [ ___------______ ]
                    //mixed
                    /*while(ith_run < runs.size() && runs[ith_run] < end_block){
                        ++ith_run;
                    }*/
                    m_full[ith_block] = 0;
                    m_info[ith_block] = 1;
                    mixed_blocks.push_back(ith_block);
                }else if(runs[ith_run] == start_block){
                    if(runs[ith_run+1] < end_block){
                        // [ --------_____ ]
                        //mixed
                        /*while(ith_run < n_blocks && runs[ith_run] < end_block){
                            ++ith_run;
                        }*/
                        m_full[ith_block] = 0;
                        m_info[ith_block] = 1;
                        mixed_blocks.push_back(ith_block);
                    }else{
                        // [ -------------- ]
                        //full
                        m_full[ith_block] = 1;
                        m_info[ith_block] = (ith_run + c[0]) % 2;
                    }

                }else{
                    // [ -------------- ]
                    //full
                    m_full[ith_block] = 1;
                    m_info[ith_block] = (ith_run + 1 + c[0]) % 2;
                }
                while(ith_run < runs.size() && runs[ith_run] < end_block){
                    ++ith_run;
                }

                start_block = end_block;
                end_block += sample;
            }
            //tricks for avoiding if statements in succ
            m_info.resize(m_info.size()+1);
            m_info[m_info.size()-1] = 1;
            m_full.resize(m_full.size()+2);
            m_full[m_full.size()-2] = 0;
            m_full[m_full.size()-1] = 1;
            //Storing info of mixed blocks
            auto aux = sdsl::bit_vector(mixed_blocks.size()*sample);
            size_type ith_mixed = 0;
            for(const auto &mixed_block : mixed_blocks){
                start_block = mixed_block * sample;
                end_block = std::min(start_block + sample, c.size());
                for(size_type i = start_block; i < end_block; ++i){
                    aux[ith_mixed] = c[i];
                    ++ith_mixed;
                }
            }
            m_mixed = mixed_bitmap_type(aux);
            sdsl::util::init_support(m_rank_full, &m_full);
        }

        //! Copy constructor
        zombit_vector_v3(const zombit_vector_v3& o)
        {
            copy(o);
        }

        //! Move constructor
        zombit_vector_v3(zombit_vector_v3&& o)
        {
            *this = std::move(o);
        }


        zombit_vector_v3 &operator=(const zombit_vector_v3 &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }
        zombit_vector_v3 &operator=(zombit_vector_v3 &&o) {
            if (this != &o) {
                m_size = o.m_size;
                m_sample = o.m_sample;
                m_full = std::move(o.m_full);
                m_rank_full = std::move(o.m_rank_full);
                m_rank_full.set_vector(&m_full);
                m_info = std::move(o.m_info);
                m_mixed = std::move(o.m_mixed);
            }
            return *this;
        }

        void swap(zombit_vector_v3 &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_size, o.m_size);
            std::swap(m_sample, o.m_sample);
            m_full.swap(o.m_full);
            sdsl::util::swap_support(m_rank_full, o.m_rank_full, &m_full, &o.m_full);
            m_info.swap(o.m_info);
            m_mixed.swap(o.m_mixed);
        }

        inline value_type access(const size_type i) const{

            //std::cout << "acess at " << i << std::endl;
            auto j = i / sample;
            if(m_full[j]) return m_info[j];
            auto i_mixed = j - m_rank_full(j+1);
            return m_mixed[i_mixed*sample + i%sample];

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
            written_bytes += m_full.serialize(out, child, "full");
            written_bytes += m_rank_full.serialize(out, child, "rank_full");
            written_bytes += m_info.serialize(out, child, "info");
            written_bytes += m_mixed.serialize(out, child, "mixed");
            written_bytes += sdsl::serialize(m_size, out, child, "size");
            written_bytes += sdsl::serialize(m_sample, out, child, "sample");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream& in)
        {
            m_full.load(in);
            m_rank_full.load(in, &m_full);
            m_info.load(in);
            m_mixed.load(in);
            sdsl::load(m_size, in);
            sdsl::load(m_sample, in);
        }

        void print(){
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
        }

    };


    template<uint8_t t_b>
    struct rank_support_zombit_v3_trait {
        typedef uint64_t size_type;
        static size_type adjust_rank(size_type r,size_type)
        {
            return r;
        }
    };

    template<>
    struct rank_support_zombit_v3_trait<0> {
        typedef uint64_t size_type;
        static size_type adjust_rank(size_type r, size_type n)
        {
            return n - r;
        }
    };


    template <uint8_t t_b=1, class t_mixed = sdsl::bit_vector>
    class rank_support_zombit_v3 {

    public:
        friend class rank_support_zombit_rec_v3<1>;
        friend class rank_support_zombit_rec_v3<0>;
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename t_mixed::rank_1_type rank_mixed_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_type;
    private:
        const zombit_vector_v3<t_mixed>* m_v = nullptr;
        rank_mixed_type m_rank_mixed;
        rank_type m_rank_info;

        void copy(const rank_support_zombit_v3& ss){
            m_v = ss.m_v;
            m_rank_mixed = ss.m_rank_mixed;
            m_rank_info = ss.m_rank_info;
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
                m_rank_info.set_vector(&(m_v->m_info));
            }
        }
    public:


        rank_support_zombit_v3() = default;

        //! Copy constructor
        rank_support_zombit_v3(const rank_support_zombit_v3& hybrid)
        {
            copy(hybrid);
        }

        explicit rank_support_zombit_v3(const zombit_vector_v3<t_mixed>* v)
        {
            m_v = v;
            sdsl::util::init_support(m_rank_info, &(m_v->m_info));
            sdsl::util::init_support(m_rank_mixed, &(m_v->m_mixed));

        }


        //! Returns the position of the i-th occurrence in the bit vector.
        size_type rank(size_type i)const
        {
            assert(i >= 0); assert(i <= m_v->size());
            if(i <= 0) return 0;
            auto j = (i-1) / m_v->sample; //block
            auto full = m_v->m_rank_full(j); // Z and O blocks before j
            auto mixed = j - full; // M blocks before j
            size_type full_ones = full-(j-m_rank_info(j));
            size_type r = full_ones * m_v->sample;
            if(m_v->m_full[j]){
                r = (m_v->m_info[j]) ? r + m_rank_mixed(mixed*m_v->sample) + ((i-1) % m_v->sample)+1 : r + m_rank_mixed(mixed*m_v->sample);
                /*if(m_v->m_info[j]){
                    // r contains all the ones in O blocks
                    // mixed*sample= position where the mixed-th mixed block ends+1
                    // (i-1)%sample position of the (i-1) bit inside the full block
                    // +1 because we are counting
                    r = r + m_rank_mixed(mixed*m_v->sample) + ((i-1) % m_v->sample)+1;
                }else{
                    //r contains all the ones in O blocks
                    // mixed*sample= position where the (mixed+1)-th M block starts
                    r = r + m_rank_mixed(mixed*m_v->sample);
                }*/
            }else{
                //r contains all the ones in O blocks
                // (i-1)%sample position of the (i-1) bit inside the block
                // +1 because we need an inclusive rank
                r = r + m_rank_mixed(mixed*m_v->sample + (i-1) % m_v->sample + 1);
            }
            return rank_support_zombit_v3_trait<t_b>::adjust_rank(r, i);
        }


        size_type operator()(size_type i)const
        {
            return rank(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const zombit_vector_v3<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                sdsl::util::init_support(m_rank_mixed, &(m_v->mixed));
                sdsl::util::init_support(m_rank_info, &(m_v->m_info));
            }

        }

        rank_support_zombit_v3& operator=(const rank_support_zombit_v3& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        rank_support_zombit_v3& operator=(rank_support_zombit_v3& ss)
        {

            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_rank_mixed = std::move(ss.m_rank_mixed);
                m_rank_info = std::move(ss.m_rank_info);
                if(m_v != nullptr){
                    m_rank_mixed.set_vector(&(m_v->mixed));
                    m_rank_info.set_vector(&(m_v->m_info));
                }
            }
            return *this;
        }

        void swap(rank_support_zombit_v3& o) {
            std::swap(m_v, o.m_v);
            m_rank_mixed.swap(o.m_rank_mixed);
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
                m_rank_info.set_vector(&(m_v->m_info));
            }else{
                m_rank_mixed.set_vector(nullptr);
                m_rank_info.set_vector(nullptr);
            }
            if(o.m_v != nullptr){
                o.m_rank_mixed.set_vector(&(o.m_v->mixed));
                o.m_rank_info.set_vector(&(o.m_v->m_info));
            }else{
                o.m_rank_mixed.set_vector(nullptr);
                o.m_rank_info.set_vector(nullptr);
            }


        }



        void load(std::istream& in, const zombit_vector_v3<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                m_rank_mixed.load(in, &(m_v->mixed));
                m_rank_info.load(in, &(m_v->m_info));
            }
        }

        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_rank_mixed.serialize(out, child, "rank_mixed");
            written_bytes += m_rank_info.serialize(out, child, "rank_info");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };



    template <uint8_t t_b, class t_mixed>
    class succ_support_zombit_v3 {

        friend class succ_support_zombit_rec_v3<1>;
        friend class succ_support_zombit_rec_v3<0>;
    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename t_mixed::succ_1_type succ_mixed_type;
    private:
        const zombit_vector_v3<t_mixed>* m_v;
        sdsl::succ_support_v<1> m_next_check;
        succ_mixed_type m_succ_mixed;


        void copy(const succ_support_zombit_v3& ss){
            m_v = ss.m_v;
            m_next_check = ss.m_next_check;
            m_succ_mixed = ss.m_succ_mixed;
            if(m_v != nullptr){
                m_next_check.set_vector(&m_v->m_info);
                m_succ_mixed.set_vector(&(m_v->m_mixed));
            }else{
                m_next_check.set_vector(nullptr);
                m_succ_mixed.set_vector(nullptr);
            }
        }

    public:

        //! Copy constructor
        succ_support_zombit_v3(const succ_support_zombit_v3& hybrid)
        {
            copy(hybrid);
        }

        succ_support_zombit_v3(const zombit_vector_v3<t_mixed>* v = nullptr)
        {
            m_v = v;
            if(m_v == nullptr) return;
            sdsl::util::init_support(m_next_check, &(m_v->m_info));
            sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
        }

        inline size_type succ(size_type i) const{
            auto j = i /m_v->sample;
            auto q = j+1-m_v->m_rank_full(j+1);
            size_type r;
            if(m_v->m_full[j]) {
                if (m_v->m_info[j]) return i;
                j = m_next_check(j+1);
                if(j >= m_v->m_info.size()-1) return m_v->size();
                r = (m_v->m_full[j]) ? j * m_v->sample :
                    j * m_v->sample + m_succ_mixed((q - 1) * m_v->sample + i % m_v->sample) - q*m_v->sample;
            }else {
                auto s_m = m_succ_mixed((q - 1) * m_v->sample + i % m_v->sample);
                if (s_m < q * m_v->sample) return s_m - (q - 1) * m_v->sample + j * m_v->sample;
                j = m_next_check(j+1);
                if(j >= m_v->m_info.size()-1) return m_v->size();
                r = (m_v->m_full[j]) ? j * m_v->sample : j  * m_v->sample + s_m - q*m_v->sample;
            }
            return r;
        };



        size_type operator()(size_type i)const
        {
            return succ(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const zombit_vector_v3<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
                sdsl::util::init_support(m_next_check, &(m_v->m_info));
            }
        }

        succ_support_zombit_v3& operator=(const succ_support_zombit_v3& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_zombit_v3& operator=(succ_support_zombit_v3&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_next_check = std::move(ss.m_next_check);
                m_succ_mixed == std::move(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_next_check.set_vector(&(m_v->m_info));
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_mixed.set_vector(nullptr);
                }
            }
            return *this;
        }

        void swap(succ_support_zombit_v3& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
                m_next_check.swap(ss.m_next_check);
                m_succ_mixed.swap(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_next_check.set_vector(&(m_v->m_info));
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_next_check.set_vector(nullptr);
                    m_succ_mixed.set_vector(nullptr);
                }
                if(ss.m_v != nullptr){
                    ss.m_succ_mixed.set_vector(&(ss.m_v->m_mixed));
                }else{
                    ss.m_succ_mixed.set_vector(nullptr);
                }
            }
        }

        void load(std::istream& in, const zombit_vector_v3<t_mixed>* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                m_succ_mixed.load(in, &(m_v->m_mixed));
                m_next_check.load(in, &(m_v->m_info));
            }
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_next_check.serialize(out, child, "m_next_check");
            written_bytes += m_succ_mixed.serialize(out, child, "succ_mixed");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };



    //Only with t_b=1
    class succ_support_zombit_v3_naive {

        friend class succ_support_zombit_rec_v3<1>;
    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
    private:
        const zombit_vector_v3<sdsl::bit_vector>* m_v;


        void copy(const succ_support_zombit_v3_naive& ss){
            m_v = ss.m_v;
        }

    public:

        //! Copy constructor
        succ_support_zombit_v3_naive(const succ_support_zombit_v3_naive& hybrid)
        {
            copy(hybrid);
        }

        succ_support_zombit_v3_naive(const zombit_vector_v3<sdsl::bit_vector>* v = nullptr)
        {
            m_v = v;
            if(m_v == nullptr) return;
        }

        inline size_type succ(size_type i) const{
            auto j = i /m_v->sample;
            auto q = j+1-m_v->m_rank_full(j+1);
            size_type r;
            if(m_v->m_full[j]) {
                if (m_v->m_info[j]) return i;
                j = sdsl::bits_more::next_limit(m_v->m_info.data(), j+1, m_v->m_info.size());
                if(j >= m_v->m_info.size()-1) return m_v->size();
                r = (m_v->m_full[j]) ? j * m_v->sample :
                        j * m_v->sample + sdsl::bits_more::next_limit(m_v->mixed.data(), q*m_v->sample,(q+1)*m_v->sample) - q*m_v->sample;
            }else {
                auto n_m = sdsl::bits_more::next_limit(m_v->mixed.data(),(q - 1) * m_v->sample + i % m_v->sample,
                                                                 (q+1)*m_v->sample);

                if (n_m < q * m_v->sample) return n_m - (q - 1) * m_v->sample + j * m_v->sample;
                j = sdsl::bits_more::next_limit(m_v->m_info.data(), j+1, m_v->m_info.size());
                if(j >= m_v->m_info.size()-1) return m_v->size();
                r = (m_v->m_full[j]) ? j * m_v->sample : j  * m_v->sample + n_m - q*m_v->sample;
            }
            return r;
        };

        size_type operator()(size_type i)const
        {
            return succ(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const zombit_vector_v3<sdsl::bit_vector>* v=nullptr)
        {
            m_v = v;
        }

        succ_support_zombit_v3_naive& operator=(const succ_support_zombit_v3_naive& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_zombit_v3_naive& operator=(succ_support_zombit_v3_naive&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
            }
            return *this;
        }

        void swap(succ_support_zombit_v3_naive& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
            }
        }

        void load(std::istream& in, const zombit_vector_v3<sdsl::bit_vector>* v=nullptr)
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
 */
    template<uint8_t t_b, class t_mixed>
    class select_support_zombit_v3
    {
    public:
        typedef zombit_vector_v3<t_mixed> bit_vector_type;
        typedef rank_support_zombit_v3<t_b, t_mixed> rank_zombit_type;
        typedef typename bit_vector_type::size_type size_type;
        enum { bit_pat = t_b };
        enum { bit_pat_len = (uint8_t)1 };
    private:
        //const bit_vector_type* m_v;
        const rank_zombit_type* m_rank;

    public:
        //! Standard constructor

        select_support_zombit_v3() = default;


        explicit select_support_zombit_v3(const zombit_vector_v3<t_mixed>* bv)
        {
        }

        explicit select_support_zombit_v3(const rank_zombit_type* rank)
        {
            m_rank = rank;
        }

        //! Answers select queries
        size_type select(size_type i) const
        {
            //fprintf(stderr, "\nzombit_vector: select queries are not currently supported\n");
            //std::exit(EXIT_FAILURE);

            auto l = 0; auto r = m_rank->size()-1;
            auto mid = (l + r) >> 1;
            while(l < r){
                auto cnt = m_rank(mid+1);
                if(cnt < i){
                    l = mid + 1;
                }else{
                    r = mid;
                }
            }
            return l;
        }

        //! Shorthand for select(i)
        size_type operator()(size_type i) const
        {
            return select(i);
        }

        //! Return the size of the original vector
        size_type size() const
        {
            return m_rank->size();
        }

        //! Assignment operator
        select_support_zombit_v3& operator=(const select_support_zombit_v3& rs)
        {
            if (this != &rs) {
                m_rank = rs.m_rank;
            }
            return *this;
        }

        //! Swap method
        void swap(select_support_zombit_v3&) {}

        //! Load the data structure from a stream and set the supported vector
        void load(std::istream&, const rank_zombit_type* rank)
        {
            m_rank = rank;
        }

        //! Load the data structure from a stream and set the supported vector
        void load(std::istream&, const zombit_vector_v3<t_mixed>* bv)
        {
        }

        void set_vector(const zombit_vector_v3<t_mixed>* bv){

        }

        //! Serializes the data structure into a stream
        size_type serialize(std::ostream&, sdsl::structure_tree_node* v = nullptr, std::string name = "") const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            sdsl::structure_tree::add_size(child, 0);
            return 0;
        }
    };

    /* template <uint8_t t_b>
    class succ_support_zombit_rapid {

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
    private:
        const zombit_vector* m_v;
        sdsl::int_vector<> m_solutions;


        void copy(const succ_support_zombit_rapid& ss){
            m_v = ss.m_v;
            m_check_block = ss.m_check_block;
            m_next_check = ss.m_next_check;
            m_next_check.set_vector(&m_check_block);
            m_succ_mixed == ss.m_succ_mixed;
            if(m_v != nullptr){
                m_succ_mixed.set_vector(&(m_v->m_mixed));
            }else{
                m_succ_mixed.set_vector(nullptr);
            }
        }

    public:

        succ_support_zombit_rapid() = default;


        explicit succ_support_zombit_rapid(const zombit_vector* v)
        {
            m_v = v;
            if(m_v == nullptr) return;
            m_check_block = sdsl::bit_vector(m_v->m_full.size(), 0);
            for(size_type i = 0; i < m_v->m_full.size(); ++i){
                if(m_v->m_full[i]){
                    if(m_v->m_full_type[m_v->m_rank_full(i+1)-1]){
                        m_check_block[i] = 1;
                    }
                }else{
                    m_check_block[i] = 1;
                }
            }
            sdsl::util::init_support(m_next_check, &m_check_block);
            sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
        }

        size_type succ(size_type i) const{
            auto j = i /m_v->sample;
            auto p = m_v->m_rank_full(j+1);
            auto q = j+1-p;
            if(m_v->m_full[j]){
                if(m_v->m_full_type[p-1] == t_b) {
                    return i;
                }
            }else{
                auto next_in_mixed = m_succ_mixed((q-1)*m_v->sample + i %m_v->sample);
                if(next_in_mixed < q*m_v->sample) {
                    return next_in_mixed - (q - 1) * m_v->sample + j * m_v->sample;
                }
            }
            j = m_next_check(j+1);
            //std::cout << "next block1: " << j << " (" << m_check_block.size() << ") " << std::endl;
            if(j == m_check_block.size()) return m_v->size();
            if(m_v->m_full[j]){
                return j * m_v->sample;
            }else{
                size_type succ_mixed = m_succ_mixed(q*m_v->sample);
                if(succ_mixed == m_v->mixed.size()) return m_v->size();
                return j * m_v->sample + succ_mixed - q*m_v->sample;
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

        void set_vector(const zombit_vector* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
            }
        }

        succ_support_zombit_rapid& operator=(const succ_support_zombit_rapid& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_zombit_rapid& operator=(succ_support_zombit_rapid&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_check_block = std::move(ss.m_check_block);
                m_next_check = std::move(ss.m_next_check);
                m_next_check.set_vector(&m_check_block);
                m_succ_mixed == std::move(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_mixed.set_vector(nullptr);
                }
            }
            return *this;
        }

        void swap(succ_support_zombit_rapid& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
                m_check_block.swap(ss.m_check_block);
                sdsl::util::swap_support(m_next_check, ss.m_next_check, &m_check_block, &ss.m_check_block);
                m_succ_mixed.swap(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_mixed.set_vector(nullptr);
                }
                if(ss.m_v != nullptr){
                    ss.m_succ_mixed.set_vector(&(ss.m_v->m_mixed));
                }else{
                    ss.m_succ_mixed.set_vector(nullptr);
                }
            }
        }

        void load(std::istream& in, const succ_support_zombit_rapid* v=nullptr)
        {
            m_v = v;
            m_check_block.load(in);
            m_next_check.load(in, &m_check_block);
            if(m_v != nullptr){
                m_succ_mixed.load(in, &(m_v->m_mixed));
            }
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_check_block.serialize(out, child, "check_block");
            written_bytes += m_next_check.serialize(out, child, "m_next_check");
            written_bytes += m_succ_mixed.serialize(out, child, "succ_mixed");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };*/


    /*
    //! Returns the position of the i-th occurrence in the bit vector.
    template<>
    typename succ_support_zombit_vector<0>::size_type succ_support_zombit_vector<0>::succ(size_type i)const
    {
        assert(i >= 0); assert(i < m_v->size());
        auto sample_index = i/m_v->sample;
        if(!m_v->sampling[sample_index]) return i;
        auto ones = m_v->rank_sampling(sample_index + 1);
        auto beg = sample_index * m_v->sample + m_v->offset[ones-1];
        auto pos = m_v->pos[ones-1];
        auto next_pos = m_v->pos[ones];
        auto length = next_pos - pos;
        if(beg > i || beg + length <= i){
            return i;
        }else{
            auto next_0_value = m_succ_values((i-beg) + pos);
            if(next_0_value < next_pos){
                return beg + (next_0_value - pos);
            }
            if((beg + length) % m_v->sample != 0){
                return beg + length;
            }
            if(next_0_value > m_v->values.size()) return m_v->size();
            auto delta = m_delta_next_0[ones-1];
            if(delta == 0) return m_v->size();
            auto new_index = sample_index + delta;
            if (!m_v->sampling[new_index]) return new_index * m_v->sample;
            auto offset = m_v->offset[ones + delta - 1];
            if(offset > 0){
                return new_index * m_v->sample;
            }else{
                pos = m_v->pos[ones + delta - 1];
                next_pos = m_v->pos[ones + delta];
                beg = new_index * m_v->sample + offset;
                if(next_0_value < next_pos){
                    return beg + (next_0_value - pos);
                }else{
                    return beg + (next_pos - pos);
                }
            }


        }
    }



    //! Returns the position of the i-th occurrence in the bit vector.
    template<>
    typename succ_support_zombit_vector<0>::size_type succ_support_zombit_vector<0>::succ(size_type i, next_info_type &next_info)const
    {
        assert(i >= 0); assert(i < m_v->size());
        auto sample_index = i/m_v->sample;
        if(!m_v->sampling[sample_index]) return i;
        size_type ones, beg, pos, next_pos, length;
        if(sample_index != next_info.sample_index || !next_info.init){
            ones = m_v->rank_sampling(sample_index + 1);
            beg = sample_index * m_v->sample + m_v->offset[ones-1];
            pos = m_v->pos[ones-1];
            next_pos = m_v->pos[ones];
            length = next_pos - pos;
            next_info = {true, i/m_v->sample, ones, beg, pos, next_pos, length};
        }else{
            ones = next_info.ones;
            beg = next_info.beg;
            pos = next_info.pos;
            next_pos = next_info.next_pos;
            length = next_info.length;
        }
        if(beg > i || beg + length <= i){
            return i;
        }else{
            auto next_0_value = m_succ_values((i-beg) + pos);
            if(next_0_value < next_pos){
                return beg + (next_0_value - pos);
            }
            if((beg + length) % m_v->sample != 0){
                return beg + length;
            }
            if(next_0_value > m_v->values.size()) return m_v->size();
            auto delta = m_delta_next_0[ones-1];
            if(delta == 0) return m_v->size();
            auto new_index = sample_index + delta;
            if (!m_v->sampling[new_index]) return new_index * m_v->sample;
            auto offset = m_v->offset[ones + delta - 1];
            if(offset > 0){
                return new_index * m_v->sample;
            }else{
                pos = m_v->pos[ones + delta - 1];
                next_pos = m_v->pos[ones + delta];
                beg = new_index * m_v->sample + offset;
                if(next_0_value < next_pos){
                    return beg + (next_0_value - pos);
                }else{
                    return beg + (next_pos - pos);
                }
            }
        }
    }
    */
}

#endif //RCT_ZOMBIT_VECTOR_HPP
