//
// Created by Adrián on 8/2/24.
//

#include <sdsl/int_vector.hpp>
#include <unordered_set>



void generate(uint64_t size, double ratio, sdsl::bit_vector &bv){

    bv = sdsl::bit_vector(size, 0);

    std::random_device r;
    std::mt19937_64 rng(r());
    std::uniform_int_distribution<uint64_t> distribution(0, bv.size()-1);
    auto dice = bind(distribution, rng);
    // populate vectors with some other bits
    std::unordered_set<uint64_t> table;
    auto n_ones = static_cast<uint64_t>((double) bv.size()*ratio);
    while(table.size() < n_ones){
        uint64_t x = dice();
        if(table.find(x) == table.end()){
            bv[x] = true;
            table.insert(x);
        }
    }
}


void generate_runs(uint64_t size, double mean_1, double stdev_1, double mean_0, double stdev_0, sdsl::bit_vector &bv){

    bv = sdsl::bit_vector(size);

    std::random_device r;
    std::mt19937_64 rngz(r());
    std::mt19937_64 rngo(r());
    std::normal_distribution<double> dis_z(mean_0, stdev_0);
    std::normal_distribution<double> dis_o(mean_1, stdev_1);
    auto dice_z = bind(dis_z, rngz);
    auto dice_o = bind(dis_o, rngo);

    double min1 = mean_1-stdev_1, max1 = mean_1+stdev_1;
    double min0 = mean_0-stdev_0, max0 = mean_0+stdev_0;
    auto int_z = [&dice_z, min0, max0]{ return static_cast<uint64_t>(std::min(std::max(dice_z(), min0), max0)); };
    auto int_o = [&dice_o, min1, max1]{ return static_cast<uint64_t>(std::min(std::max(dice_o(), min1), max1)); };

    uint64_t i = 0;
    bool value = false;
    while (i < size){
        uint64_t r_size;
        r_size = (value) ? int_o() : int_z();
        //std::cout << "run of " << (uint64_t) value << "s with length=" << r_size << std::endl;
        for (uint64_t j = 0; j < r_size; ++j) {
            bv[i++] = value;
            if (i >= bv.size()) break;
        }
        value = !value;
    }

}

void stats_vector(const sdsl::bit_vector &bv){
    if (bv.empty()) return;
    uint64_t cnt_1s = 0, cnt_r1s = 0, cnt_r0s = 0;
    bool run1 = bv[0];
    for(uint64_t i = 0; i < bv.size(); ++i){
        bool v = bv[i];
        //std::cout << v << ", ";
        if(v) ++cnt_1s;
        if(v != run1){
            if(run1) ++cnt_r1s; else ++cnt_r0s;
            run1 = !run1;
        }
    }
    if(run1) ++cnt_r1s; else ++cnt_r0s;
    //std::cout << std::endl << std::endl;

    std::cout << "Stats" << std::endl;
    std::cout << " - Ones: " << cnt_1s << std::endl;
    std::cout << " - Zeroes: " << (bv.size()-cnt_1s) << std::endl;
    std::cout << " - Avg(len-r1): " << cnt_1s/(double) cnt_r1s << std::endl;
    std::cout << " - Avg(len-r0): " << (bv.size()-cnt_1s)/(double) cnt_r0s << std::endl;
    std::cout << std::endl;
}

//Bitvectors of sizes 10^7, 10^8 and 10^9 with different percentages of runs.
// - R: [1,2...10]
// - R: [20,30...90]
void exp1(){
    std::vector<double> ratios = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09,
                                  0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
    std::vector<uint64_t> sizes = {10000000, 100000000, 1000000000};

    for(uint64_t s : sizes){
        for(double r : ratios){
            sdsl::bit_vector bv;
            generate(s, r, bv);
            uint64_t ratio = r * 100;
            std::string file_name = "bit-vector-exp1." + std::to_string(s) + "." + std::to_string(ratio) + ".bin";
            sdsl::store_to_file(bv, file_name);
            std::cout << file_name << std::endl;
            stats_vector(bv);
        }
    }
}

void exp2(){
    std::vector<uint64_t> sizes = {10000000, 100000000, 1000000000};
    //std::vector<uint64_t> sizes = {10000000};

    uint64_t mean, stdev;
    for(uint64_t s : sizes){
        mean = 10;
        stdev = 5;
        while(mean < s){
            sdsl::bit_vector bv;
            generate_runs(s, mean, stdev , mean, stdev, bv);
            std::string file_name = "bit-vector-exp2.equal." + std::to_string(s) + "." + std::to_string(mean) + "." + std::to_string(stdev) + ".bin";
            sdsl::store_to_file(bv, file_name);
            std::cout << file_name << std::endl;
            stats_vector(bv);
            mean *= 10;
            stdev *= 10;
        }
    }
    for(uint64_t s : sizes){
        mean = 10;
        stdev = 5;
        while(mean < s){
            sdsl::bit_vector bv;
            auto mean_1 = std::max(1.0, (double) mean/8);
            auto stdev_1 = std::max(1.0, (double) stdev/8);
            generate_runs(s, mean_1, stdev_1 , mean, stdev, bv);
            std::string file_name = "bit-vector-exp2.notequal." + std::to_string(s) + "." + std::to_string(mean) + "." + std::to_string(stdev) + ".bin";
            sdsl::store_to_file(bv, file_name);
            std::cout << file_name << std::endl;
            stats_vector(bv);
            mean *= 10;
            stdev *= 10;
        }
    }
}

void help(const std::string &ex){
    std::cout << ex << " <exp>" << std::endl;
    std::cout << "<exp> can take values 1 (exp1) and 2 (exp2)." << std::endl;
}

int main(int argc, char* argv[])
{
   /* sdsl::bit_vector bv_sparse, bv_dense, bv_runs;
    generate(1000, 0.05, bv_sparse);
    generate(1000, 0.85, bv_dense);
    generate_runs(1000, 100, 15, 50, 10, bv_runs);

    std::cout << "Sparse" << std::endl;
    print_vector(bv_sparse);

    std::cout << "Dense" << std::endl;
    print_vector(bv_dense);

    std::cout << "Runs" << std::endl;
    print_vector(bv_runs);*/

   if(argc != 2){
       help(argv[0]);
   }else{
       int exp = std::atoi(argv[1]);
       if(exp == 1){
           exp1();
       }else if (exp == 2){
           exp2();
       }else{
           help(argv[0]);
           std::cout << "Exp " << exp << " is not supported.";
       }
   }
}
