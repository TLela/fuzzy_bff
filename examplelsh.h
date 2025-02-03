#ifndef EXAMPLELSH_H
#define EXAMPLELSH_H

#include "lsh.h"
#include <stdlib.h>

class ExampleLSH : public LSH<uint64_t, uint64_t> {
public:
    ExampleLSH() : LSH<uint64_t, uint64_t>() {}
    ~ExampleLSH() {}

    uint64_t hashed(const uint64_t& input) override {
        if(input % 2 != 0){
            return input-1;
        } else {
            return input;
        }
    }
};


#endif // EXAMPLELSH_H
