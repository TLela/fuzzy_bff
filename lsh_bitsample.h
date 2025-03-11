#ifndef BITSAMPLELSH_H
#define BITSAMPLELSH_H

#include "lsh.h"
#include <stdlib.h>

class BitSampleLSH : public LSH<uint64_t, uint64_t> {
public:
    BitSampleLSH() : LSH<uint64_t, uint64_t>() {
        //change variables according to use case
        r_1 = 0.05;
        r_2 = 0.4;
        t = 0.2; //TODO: needed? redifine fp/fn?
        and_op = 35; //increase to decrease fp
        or_op = 45; //increase to decrease fn

        //resulting probabilities
        p_1 = 1 - pow(1 - pow(1 - r_1, and_op) , or_op);
        p_2 = 1 - pow(1 - pow(1 - r_2, and_op) , or_op);

        std::cout << "p_1: " << p_1 << std::endl;
        std::cout << "p_2: " << p_2 << std::endl;

        //sample bitmasks
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> bits(0, 63);

        std::unordered_set<int> flipped_bits; 
        uint64_t bitmask;
        for(int i = 0; i < or_op; i++){
            while(flipped_bits.size() < and_op){
                flipped_bits.insert(bits(gen));
            }
            bitmask = 0;
            for (int flipped_bit : flipped_bits) {
                bitmask ^= (1ULL << flipped_bit);
            }  
            bitmasks.push_back(bitmask);
            flipped_bits.clear();   
        }
    }
    ~BitSampleLSH() {}

    std::vector<uint64_t> hash_values(const uint64_t& input) override {
        std::vector<uint64_t> result;
        for(int i = 0; i < or_op; i++){
            result.push_back(input & bitmasks[i]);
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
    std::vector<uint64_t> bitmasks;
};

#endif // BITSAMPLELSH_H
