#ifndef BITSAMPLELSH_H
#define BITSAMPLELSH_H

#include "lsh.h"
#include <stdlib.h>

class BitSampleLSH : public LSH<uint64_t, uint64_t> {
public:
    BitSampleLSH() : LSH<uint64_t, uint64_t>() {
        //change variables according to use case
        r_1 = 3;
        r_2 = 15;
        and_op = 56; //increase to decrease fp; can be at most 56
        or_op = 160; //increase to decrease fn; can be at most 256
        k = 64;

        //resulting probabilities
        p_1 = 1 - pow(1 - pow(1 - r_1/k, and_op) , or_op);
        p_2 = 1 - pow(1 - pow(1 - r_2/k, and_op) , or_op);

        //sample bitmasks
        std::random_device rd;
        std::mt19937_64 gen(rand());
        std::uniform_int_distribution dis(0,63);

        for(int i = 0; i < or_op; i++){
            std::vector<int> bitmask(64,0);
            for (int j = 0; j < and_op; j++) {
                bitmask[dis(gen)] = 1;
            } 
            bitmasks.push_back(bitmask);
        }
    }
    ~BitSampleLSH() {}

    std::vector<uint64_t> hash_values(const uint64_t& input) override {
        std::vector<uint64_t> result;
        for(int i = 0; i < or_op; i++){
            uint64_t msb_value = static_cast<uint64_t>(i) << 56;
            std::bitset<64> bit_input(input);
            std::bitset<64> hash(0);
            int andpos = 0;
            for(int j = 0; j < 64; j++){
                if(bitmasks[i][j]){
                    hash.set(andpos, bit_input[j]);
                    andpos++;
                }
            }
            uint64_t hash_value = hash.to_ullong() | msb_value;
            result.push_back(hash_value);
        }
        return result;
    }

    int and_op;
    int or_op;
    double r_1;
    double r_2;
    double p_1;
    double p_2;
    double t;
    int k;
    std::vector<std::vector<int>> bitmasks;
};

#endif // BITSAMPLELSH_H
