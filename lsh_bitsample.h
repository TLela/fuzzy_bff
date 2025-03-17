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
        and_op = 35; //increase to decrease fp
        or_op = 45; //increase to decrease fn
        //or_op = int(std::floor(and_op/pow(1-r_1,and_op))); //increase to decrease fn

        //resulting probabilities
        p_1 = 1 - pow(1 - pow(1 - r_1, and_op) , or_op);
        p_2 = 1 - pow(1 - pow(1 - r_2, and_op) , or_op);

        std::cout << "and_op: " << and_op << std::endl;
        std::cout << "or_op: " << or_op << std::endl;
        std::cout << "p_1: " << p_1 << std::endl;
        std::cout << "p_2: " << p_2 << std::endl;

        //sample bitmasks
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> bits(0, 63);

        uint64_t bitmask;
        std::vector<int> bit_positions(64);
        std::iota(bit_positions.begin(), bit_positions.end(), 0);
        for(int i = 0; i < or_op; i++){
            // Shuffle and take the first 'and_op' bits
            std::shuffle(bit_positions.begin(), bit_positions.end(), gen);
            
            bitmask = 0;
            for (int j = 0; j < and_op; j++) {
                bitmask ^= (1ULL << bit_positions[j]);
            } 
            bitmasks.push_back(bitmask);
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
