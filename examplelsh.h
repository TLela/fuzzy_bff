#ifndef EXAMPLELSH_H
#define EXAMPLELSH_H

#include "lsh.h"
#include <stdlib.h>

class ExampleLSH : public LSH<uint64_t, uint64_t> {
public:
    ExampleLSH() : LSH<uint64_t, uint64_t>() {}
    ~ExampleLSH() {}

    std::vector<uint64_t> hash_values(const uint64_t& input) override {
        std::vector<uint64_t> result;
        if(input % 2 != 0){
            result.push_back(input-1);
        } else {
            result.push_back(input);
        }
        return result;
    }

};


#endif // EXAMPLELSH_H
