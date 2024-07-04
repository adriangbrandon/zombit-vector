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

#ifndef RUNS_VECTORS_REC_ZOMBIT_VECTOR_V3_HPP
#define RUNS_VECTORS_REC_ZOMBIT_VECTOR_V3_HPP

#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support.hpp>
#include <algorithm>
#include <util.hpp>
#include <cstdint>
#include "succ_support_v.hpp"
#include <sdsl/select_support.hpp>
#include <zombit_vector_v3.hpp>

namespace runs_vectors {

   // template<uint8_t t_b = 1> class rank_support_zombit_rec_v3;
   // template<uint8_t t_b = 1> class succ_support_zombit_rec_v3;

    class rec_zombit_vector_v3 {

    public:
        typedef sdsl::bit_vector bitmap_type;
        typedef sdsl::bit_vector values_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_bitmap_type;
        typedef typename sdsl::bit_vector::size_type size_type;
        typedef typename sdsl::bit_vector::value_type value_type;

        friend class rank_support_zombit_rec_v3<1>;
        friend class rank_support_zombit_rec_v3<0>;
        friend class succ_support_zombit_rec_v3<1>;
        friend class succ_support_zombit_rec_v3<0>;
        /*friend class rec_succ_support_zombit_naive<1>;
        friend class rec_succ_support_zombit_naive<0>;
        friend class rec_succ_support_zombit<1>;
        friend class rec_succ_support_zombit<0>;
        friend class rec_succ_support_zombit_lite<1>;
        friend class rec_succ_support_zombit_lite<0>;*/
    private:

        size_type m_size = 0;
        size_type m_levels = 0;
        std::vector<zombit_vector_v3> m_zombits;




        void copy(const rec_zombit_vector_v3 &o){
            m_size = o.m_size;
            m_levels = o.m_levels;
            m_zombits = o.m_zombits;
        }




    public:

        const size_type &levels = m_levels;

        rec_zombit_vector_v3(){
        }

        rec_zombit_vector_v3(const sdsl::bit_vector &c, const size_type max_levels = 1){
            if(c.empty()) return;
            m_size = c.size();
            sdsl::bit_vector aux_c = c;
            size_type l = 1;
            /*while(!aux_c.empty() && l < max_levels){
                compute_zom(aux_c, l);
                ++l;
            }*/
            auto zom = zombit_vector_v3(c);
            size_type size_plain;
            do {
                m_zombits.push_back(zom);
                size_plain = sdsl::size_in_bytes(zom.mixed);
                zom = zombit_vector_v3(zom.mixed);
                ++l;
            }while(size_plain > sdsl::size_in_bytes(zom) && l <= max_levels);
            m_levels = l-1;
            for(size_type i = 0; i < m_levels-1; ++i){
                sdsl::util::clear(m_zombits[i].m_mixed);
            }
        }

        //! Copy constructor
        rec_zombit_vector_v3(const rec_zombit_vector_v3& o)
        {
            copy(o);
        }

        //! Move constructor
        rec_zombit_vector_v3(rec_zombit_vector_v3&& o)
        {
            *this = std::move(o);
        }


        rec_zombit_vector_v3 &operator=(const rec_zombit_vector_v3 &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }
        rec_zombit_vector_v3 &operator=(rec_zombit_vector_v3 &&o) {
            if (this != &o) {
                m_levels = std::move(o.m_levels);
                m_size =  std::move(o.m_size);
                m_zombits =  std::move(o.m_zombits);
            }
            return *this;
        }

        void swap(rec_zombit_vector_v3 &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_size, o.m_size);
            std::swap(m_levels, o.m_levels);
            std::swap(m_zombits, o.m_zombits);
        }

        inline value_type access(const size_type i) const{

            //std::cout << "acess at " << i << std::endl;
            size_type aux_i = i;
            for(size_type l = 0; l < m_levels; ++l){
                const zombit_vector_v3& zombit = m_zombits[l];
                auto j = aux_i / zombit.sample;
                if(m_zombits[l].m_full[j]){
                    //std::cout << "full: " << j << "(" << m_full.size() << ")" << std::endl;
                    //std::cout << "rank: " << m_rank_full(j+1) << std::endl;
                    //std::cout << "value: " << m_full_type[m_rank_full(j+1)-1] << std::endl;
                    return zombit.m_info[j];
                }else{
                    auto i_mixed = j - zombit.m_rank_full(j+1);
                    aux_i = i_mixed*zombit.sample + aux_i%zombit.sample;
                    //std::cout << "mixed: " << i_mixed << std::endl;
                    //return m_mixed[i_mixed*sample + i%sample];
                }
            }
            return m_zombits[m_levels-1].m_mixed[aux_i];


        }

        inline value_type operator[](const size_type i) const{
            return access(i);
        }

        inline size_type size() const {
            return m_size;
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {

            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += sdsl::serialize(m_size, out, child, "size");
            written_bytes += sdsl::serialize(m_levels, out, child, "levels");
            written_bytes += sdsl::serialize_vector(m_zombits, out, child, "zombits");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream& in)
        {
            sdsl::load(m_size, in);
            sdsl::load(m_levels, in);
            m_zombits.resize(m_levels);
            sdsl::load_vector(m_zombits, in);
        }

        /*void print(){
            std::cout << "sample=" << sample << std::endl;
            std::cout << "full={";
            for(auto i = 0; i < m_full.size(); ++i){
                std::cout << m_full[i] << ", ";
            }
            std::cout << "}" << std::endl;

            std::cout << "full_type={";
            for(auto i = 0; i < m_full_type.size(); ++i){
                std::cout << m_full_type[i] << ", ";
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
    struct rank_support_zombit_rec_v3_trait {
        typedef rec_zombit_vector_v3::size_type size_type;
        static size_type adjust_rank(size_type r,size_type)
        {
            return r;
        }
    };

    template<>
    struct rank_support_zombit_rec_v3_trait<0> {
        typedef rec_zombit_vector_v3::size_type size_type;
        static size_type adjust_rank(size_type r, size_type n)
        {
            return n - r;
        }
    };


    template<uint8_t t_b>
    class rank_support_zombit_rec_v3 {

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_mixed_type;
    private:
        const rec_zombit_vector_v3* m_v = nullptr;
        std::vector<rank_support_zombit_v3<t_b>> m_rank_zombit;
        rank_mixed_type m_rank_mixed;

        void copy(const rank_support_zombit_rec_v3& ss){
            m_v = ss.m_v;
            m_rank_mixed = ss.m_rank_mixed;
            m_rank_zombit = ss.m_rank_zombit;
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->m_zombits[m_v->levels-1].m_mixed));
                container_set_vector(m_v->m_zombits, m_rank_zombit);
            }
        }
    public:


        rank_support_zombit_rec_v3(){};

        rank_support_zombit_rec_v3(const rec_zombit_vector_v3* v)
        {
            m_v = v;
            if(m_v != nullptr){
                m_rank_zombit.resize(m_v->levels);
                sdsl::util::init_support(m_rank_mixed, &(m_v->m_zombits[m_v->levels-1].m_mixed));
                container_init_support(m_v->m_zombits, m_rank_zombit);
            }

        }


        //! Returns the position of the i-th occurrence in the bit vector.
        size_type rank(size_type i)const
        {
            assert(i >= 0); assert(i <= m_v->size());
            if(i <= 0) return 0;

            //std::cout << "start " << std::endl;
            auto aux_i = i;
            size_type r = 0;
            for(size_type l = 0; l < m_v->m_levels; ++l){
                const zombit_vector_v3& zombit = m_v->m_zombits[l];
                //std::cout << "aux_i: " << aux_i << std::endl;
                if(aux_i == 0) return rank_support_zombit_rec_v3_trait<t_b>::adjust_rank(r, i);
                auto j = (aux_i-1) / zombit.sample; //block
                auto full = zombit.m_rank_full(j); // Z and O blocks before j
                auto mixed = j - full; // M blocks before j
                size_type full_ones = full-(j-m_rank_zombit[l].m_rank_info(j));
                r = full_ones * zombit.sample;
                if(zombit.m_full[j]){
                    if(zombit.m_info[full]){
                        // r contains all the ones in O blocks
                        // mixed*sample= position where the mixed-th mixed block ends+1
                        // (i-1)%sample position of the (i-1) bit inside the full block
                        // +1 because we are counting
                        r = r + ((aux_i-1) % zombit.sample)+1;
                        //std::cout << "a: " << r << std::endl;
                    }
                    //std::cout << "b: " << r << std::endl;
                    aux_i = mixed*zombit.sample;
                }else{
                    //r contains all the ones in O blocks
                    // (i-1)%sample position of the (i-1) bit inside the block
                    // +1 because we need an inclusive rank
                    aux_i = mixed*zombit.sample + ((aux_i-1) % zombit.sample) + 1;
                    //std::cout << "c: " << r << std::endl;
                }

            }
            r = r + m_rank_mixed(aux_i);
            //std::cout << "result: " << r << std::endl;
            return rank_support_zombit_rec_v3_trait<t_b>::adjust_rank(r, i);
        }


        size_type operator()(size_type i)const
        {
            return rank(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const rec_zombit_vector_v3* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                m_rank_zombit.resize(m_v->levels);
                sdsl::util::init_support(m_rank_mixed, &(m_v->m_zombits[m_v->levels-1].m_mixed));
                container_init_support(m_v->m_zombits, m_rank_zombit);
            }

        }

        //! Copy constructor
        rank_support_zombit_rec_v3(const rank_support_zombit_rec_v3& o)
        {
            copy(o);
        }

        //! Move constructor
        rank_support_zombit_rec_v3(rank_support_zombit_rec_v3&& o)
        {
            *this = std::move(o);
        }

        rank_support_zombit_rec_v3& operator=(const rank_support_zombit_rec_v3& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        rank_support_zombit_rec_v3& operator=(rank_support_zombit_rec_v3& ss)
        {

            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_rank_zombit = std::move(ss.m_rank_zombit);
                m_rank_mixed = std::move(ss.m_rank_mixed);
                if(m_v != nullptr){
                    sdsl::util::init_support(m_rank_mixed, &(m_v->m_zombits[m_v->levels-1].m_mixed));
                    container_init_support(m_v->m_zombits, m_rank_zombit);
                }
            }
            return *this;
        }

        void swap(rank_support_zombit_rec_v3& o) {
            std::swap(m_v, o.m_v);
            m_rank_zombit.swap(o.m_rank_zombit);
            m_rank_mixed.swap(o.m_rank_mixed);
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->m_zombits[m_v->levels-1].m_mixed));
                container_set_vector(m_v->m_zombits, m_rank_zombit);
            }else{
                m_rank_mixed.set_vector(nullptr);
                container_set_vector_null(m_rank_zombit);
            }
            if(o.m_v != nullptr){
                o.m_rank_mixed.set_vector(&(o.m_v->m_zombits[m_v->levels-1].m_mixed));
                container_set_vector(o.m_v->m_zombits, o.m_rank_zombit);
            }else{
                o.m_rank_mixed.set_vector(nullptr);
                container_set_vector_null(o.m_rank_zombit);
            }
        }

        void load(std::istream& in, const rec_zombit_vector_v3* v=nullptr)
        {
            m_v = v;
            m_rank_mixed.load(in);
            if(v != nullptr){
                m_rank_zombit.resize(m_v->levels);
                sdsl::load_vector(m_rank_zombit, in);
                m_rank_mixed.set_vector(&(m_v->m_zombits[m_v->levels-1].m_mixed));
                container_set_vector(m_v->m_zombits, m_rank_zombit);
            }
        }

        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += sdsl::serialize_vector(m_rank_zombit, out, child, "rank_zombit");
            written_bytes +=  m_rank_mixed.serialize(out, child, "rank_mixed");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };




        template <uint8_t t_b>
        class succ_support_zombit_rec_v3 {

        public:
            typedef sdsl::bit_vector::size_type size_type;
            typedef sdsl::bit_vector::value_type value_type;
        private:
            const rec_zombit_vector_v3* m_v = nullptr;
            std::vector<succ_support_zombit_v3_naive<t_b>> m_succ;


            void copy(const succ_support_zombit_rec_v3& ss){
                m_v = ss.m_v;
                if (m_v != nullptr) {
                    m_succ.resize(m_v->levels);
                    container_set_vector(m_v->m_zombits, m_succ);
                }
            }

        public:

            succ_support_zombit_rec_v3() = default;


            succ_support_zombit_rec_v3(const rec_zombit_vector_v3* v)
            {
                m_v = v;
                if(m_v != nullptr){
                    m_succ = std::vector<succ_support_zombit_v3_naive<t_b>>(m_v->levels, succ_support_zombit_v3_naive<t_b>());
                    container_init_support(m_v->m_zombits, m_succ);
                }
            }

            size_type rec_succ(const size_type i, const size_type level) const{
                const zombit_vector_v3& zombit = m_v->m_zombits[level];
                size_type j = i/zombit.sample;
                size_type r =0;
                if(zombit.m_full[j] && zombit.m_info[j] == t_b) {
                        return i;
                }else{
                    auto p = zombit.m_rank_full(j+1);
                    auto q = j+1-p;
                    if(level == m_v->m_levels-1){
                        if(zombit.m_full[j]){
                            j = m_succ[level].m_next_check(j+1);
                            //std::cout << "next block1: " << j << " (" << m_check_block.size() << ") " << std::endl;
                            if(j == zombit.m_info.size()-1) return zombit.size();
                            if(zombit.m_full[j]){
                                return j * zombit.sample;
                            }else{
                                size_type succ_mixed = m_succ[level].m_succ_mixed(q*zombit.sample);
                                if(succ_mixed == zombit.mixed.size()) return zombit.size();
                                return j * zombit.sample + succ_mixed - q*zombit.sample;
                            }
                        }else{
                            size_type o = m_succ[level].m_succ_mixed((q-1)*zombit.sample + (i % zombit.sample));
                            if(o < q*zombit.sample){
                                return o-(q-1)*zombit.sample+j*zombit.sample;
                            }else{
                                j = m_succ[level].m_next_check(j+1);
                                //std::cout << "next block1: " << j << " (" << m_check_block.size() << ") " << std::endl;
                                if(j == zombit.m_info.size()-1) return zombit.size();
                                if(zombit.m_full[j]){
                                    return j * zombit.sample;
                                }else{
                                    size_type succ_mixed = m_succ[level].m_succ_mixed(q*zombit.sample);
                                    if(succ_mixed == zombit.mixed.size()) return zombit.size();
                                    return j * zombit.sample + succ_mixed - q*zombit.sample;
                                }
                            }
                        }
                    }else{

                        //Empty->next
                        if(zombit.m_full[j]){
                             j = m_succ[level].m_next_check(j+1);
                            if(j == zombit.m_info.size()-1) return zombit.size();
                            if(zombit.m_full[j]){
                                return j*zombit.sample;
                            }else{
                                size_type succ_mixed = rec_succ(q*zombit.sample, level+1);
                                if(succ_mixed == m_v->m_zombits[level+1].size()) return zombit.size();
                                return j * zombit.sample + succ_mixed - q*zombit.sample;
                            }
                        }else{
                            size_type aux_i = (q-1) * zombit.sample + (i % zombit.sample);
                            r = rec_succ(aux_i, level+1);
                            if(r < q*zombit.sample){
                                return r - (q-1)*zombit.sample + j*zombit.sample;
                            }else{
                                j = m_succ[level].m_next_check(j+1);
                                if(j == zombit.m_info.size()-1) return zombit.size();
                                if(zombit.m_full[j]){
                                    return j*zombit.sample;
                                }else{
                                    size_type succ_mixed = rec_succ(q*zombit.sample, level+1);
                                    if(succ_mixed == m_v->m_zombits[level+1].size()) return zombit.size();
                                    return j * zombit.sample + succ_mixed - q*zombit.sample;
                                }
                            }
                        }
                    }
                }
                return r;


            }

            /*size_type rec_succ(const size_type i, const size_type level) const{

                auto sample = m_v->sample[level];
                auto j = i / sample;
                //1. Check if there is information in a M-block
                if(j >= m_v->m_full[level].size()) return m_v->size();
                auto k = j ? m_v->m_rank_full[level](j) : 0;
                if(m_v->m_full[level][j] && m_v->m_full_type[level][k]){
                    //2. The one is in a O-block
                    return i;
                }else{
                    //3. Next O-block
                    size_type p_full_ones = m_v->size();
                    size_type n_f = m_succ_full_type[level](k);
                    if(n_f < m_v->m_full_type[level].size()){
                        p_full_ones = m_select_full[level](n_f+1)*sample;
                    }
                    //4. Next M-block in the last level
                    size_type p_mixed = m_v->size();
                    if(level == m_v->m_levels-1){
                        auto mixed_blocks = j-k;
                        auto start_mixed_block = mixed_blocks*sample;
                        if(start_mixed_block < m_v->mixed.size()){
                            //4.1 From a M-block
                            if(!m_v->m_full[level][j]){
                                auto new_mixed_block = start_mixed_block + sample;
                                auto next_in_mixed = m_succ_mixed(start_mixed_block + i%sample);
                                if(next_in_mixed < new_mixed_block){
                                    p_mixed = next_in_mixed - start_mixed_block + j*sample;
                                }else if (j + 1 < m_v->m_full[level].size()) {
                                    start_mixed_block = new_mixed_block;
                                    n_f = m_succ_no_full[level](j + 1);
                                    p_mixed = n_f * sample + (next_in_mixed - start_mixed_block);
                                }
                            }else{
                                //2.2 From a Z-block
                                if (j + 1 < m_v->m_full[level].size()) {
                                    n_f = m_succ_no_full[level](j + 1);
                                    p_mixed = n_f * sample + (m_succ_mixed(start_mixed_block) - start_mixed_block);
                                }
                            }
                        }
                    }else{
                        //5. Next M-block in all the levels except the last one
                        auto mixed_blocks = j-k;
                        size_type aux_i = mixed_blocks * sample;
                        if(!m_v->m_full[level][j]) {
                            aux_i += (i % sample);
                        }
                        p_mixed = rec_succ(aux_i, level+1);
                        if(m_v->m_full[level][j] || p_mixed >= (mixed_blocks+1)*sample){
                            auto diff = m_succ_no_full[level](j+1)-j-1 + m_v->m_full[level][j];
                            p_mixed += (diff * sample);
                        }
                        p_mixed += k*sample;

                    }
                    //6. Return the closest one
                    if(p_mixed > p_full_ones){
                        return p_full_ones;
                    }else {
                        return p_mixed;
                    }
                }
            }*/

            size_type succ(size_type i) const{
                bool skip = false;
                return std::min(rec_succ(i, 0), m_v->size());
            };

            size_type operator()(size_type i)const
            {
                return succ(i);
            }

            size_type size()const
            {
                return m_v->size();
            }

            void set_vector(const rec_zombit_vector_v3* v=nullptr)
            {
                m_v = v;
                if(v != nullptr){
                    container_init_support(m_v->m_zombits, m_succ);
                }
            }

            //! Copy constructor
            succ_support_zombit_rec_v3(const succ_support_zombit_rec_v3& o)
            {
                copy(o);
            }

            //! Move constructor
            succ_support_zombit_rec_v3(succ_support_zombit_rec_v3&& o)
            {
                *this = std::move(o);
            }

            succ_support_zombit_rec_v3& operator=(const succ_support_zombit_rec_v3& ss)
            {
                if (this != &ss) {
                    copy(ss);
                }
                return *this;
            }

            succ_support_zombit_rec_v3& operator=(succ_support_zombit_rec_v3&& ss)
            {
                if (this != &ss) {
                    m_v = std::move(ss.m_v);
                    m_succ = std::move(ss.m_succ);
                    if(m_v != nullptr){
                        container_set_vector(m_v->m_zombits, m_succ);
                    }
                }
                return *this;
            }

            void swap(succ_support_zombit_rec_v3& ss) {
                if (this != &ss) {
                    std::swap(m_v, ss.m_v);
                    m_succ.swap(ss.m_succ);
                    if(m_v != nullptr){
                        container_set_vector(m_v->m_zombits, m_succ);
                    }else{
                        container_set_vector_null(m_succ);
                    }
                    if(ss.m_v != nullptr){
                        container_set_vector(ss.m_v->m_zombits, ss.m_succ);
                    }else{
                        container_set_vector_null(ss.m_succ);
                    }
                }
            }

            void load(std::istream& in, const succ_support_zombit_rec_v3* v=nullptr)
            {
                m_v = v;
                if(m_v != nullptr){
                    m_succ.resize(m_v->m_levels);
                    sdsl::load_vector(m_succ, in);
                    container_set_vector(m_v->m_zombits, m_succ);
                }
            }

            //! Serializes the data structure into the given ostream
            size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
            {
                sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
                size_type written_bytes = 0;
                written_bytes += sdsl::serialize_vector(m_succ, out, child, "succ");
                sdsl::structure_tree::add_size(child, written_bytes);
                return written_bytes;
            }


        };

}

#endif //RCT_ZOMBIT_VECTOR_HPP
