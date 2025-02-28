#ifndef EXAMPLELSH2_H
#define EXAMPLELSH2_H

#include "lsh.h"
#include <stdlib.h>

class ExampleLSH2 : public LSH<uint32_t, uint64_t> {
public:
    ExampleLSH2() : LSH<uint32_t, uint64_t>() {}
    ~ExampleLSH2() {}

    std::vector<uint64_t> hash_values(const uint32_t& input) override {
        std::vector<uint64_t> result;
        if(input % 2 != 0){
            result.push_back(input-1);
        } else {
            result.push_back(input);
        }
        return result;
    }
};


#endif // EXAMPLELSH2_H
