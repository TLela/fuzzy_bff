#ifndef EXAMPLELSH2_H
#define EXAMPLELSH2_H

#include "lsh.h"
#include <stdlib.h>

class ExampleLSH2 : public LSH<uint32_t, uint64_t> {
public:
    ExampleLSH2() : LSH<uint32_t, uint64_t>() {}
    ~ExampleLSH2() {}

    uint64_t hashed(const uint32_t& input) override {
        if(input % 2 != 0){
            return input-1;
        } else {
            return input;
        }
    }
};


#endif // EXAMPLELSH2_H
