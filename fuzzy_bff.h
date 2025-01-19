#ifndef FUZZY_BFF_H
#define FUZZY_BFF_H

// Include necessary standard libraries
#include <stdlib.h>
#include <vector>
#include "hashfunction.h"


//next up: seems to be a lot of repetition in results of getHashFromHash (check on that, leads to failed construction - maybe just small values?)
// now that we have hashfunctions we can implement the populate functions step by step

using namespace std;

template <typename ItemType, typename FingerprintType, typename HashFamily>
class fuzzyBFF {
public:
    // Constructor
    fuzzyBFF(const size_t size){
        // Calculate all necessary parameters to setup filter
        this->size = size;
        this->segmentLength = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // Filter needs to be bigger than size of input set
        double factor = fmax(1.125, 0.875 + 0.25 * log(1000000) / log(size));
        this->filterLength = factor * size;

        // We need to fit an integer number of segments in the filter
        this->segmentCount = ((this->filterLength + this->segmentLength - 1) / this->segmentLength);

        // TODO: decide what to do for small sizes. what if less than 3 segments are needed?

        // Size of the logical filter array
        this->filterLength = this->segmentCount * this->segmentLength;
        // We wrap around the last two segments to ensure uniform randomness of hash positions of elements
        // This is done by adding two additional segments at the end of the filter - these correspond to the first two segments
        this->segmentCount += 2;
        this->arrayLength = this->segmentCount * this->segmentLength;

        // Allocate memory for filter
        this->filter = new FingerprintType[this->filterLength]();
        std::fill_n(this->filter, this->filterLength, 0);

        // Initialize hash function
        this->hashfunction = new HashFamily();
    }

    // Destructor
    ~fuzzyBFF();

    // Public member functions
    
    // Populate with data in vector data
    bool populate(const vector<ItemType>& data, size_t length){
        return  populate(data.data(), length);
    }

    // Populate with data in array data
    bool populate(const ItemType* data, size_t length);

    // Get pointer to filter
    FingerprintType* getFilter(){

        return this->filter;
    }

    
    // See https://github.com/FastFilter/fastfilter_cpp
    inline __attribute__((always_inline)) size_t getHashFromHash(uint64_t hash,
                                                               int index) {
        __uint128_t x = (__uint128_t)hash * (__uint128_t)(this->segmentCount*this->segmentLength);
        uint64_t h = (uint64_t)(x >> 64);
        h += index * this->segmentLength;
        // keep the lower 36 bits
        uint64_t hh = hash & ((1UL << 36) - 1);
        // index 0: right shift by 36; index 1: right shift by 18; index 2: no shift
        h ^= (size_t)((hh >> (36 - 18 * index)) & (this->segmentLength - 1));
        return h;
    }

    bool membership();

    // Public member variables
    // TODO: make private after testing?
    size_t size;
    size_t segmentLength;
    size_t arrayLength;
    size_t filterLength;
    size_t segmentCount;
    HashFamily *hashfunction;

   
private:
    // Private member variables
    
    // Filter array
    FingerprintType* filter;


    // Private member functions

};


template <typename ItemType, typename FingerprintType, typename HashFamily>
bool fuzzyBFF<ItemType, FingerprintType, HashFamily>::populate(const ItemType* data, size_t length){

    // Check if the filter is big enough to hold the data
    if (length > this->size) {
        return false;
    }

    // Initialize arrays for insertion algorithm


    // populate filter trivial test
    for (size_t i = 0; i < length; i++) {
        uint64_t hash = (*hashfunction)(data[i]);
        this->filter[3*i] = getHashFromHash(hash, 0);
        this->filter[3*i+1] = getHashFromHash(hash, 1);
        this->filter[3*i+2] = getHashFromHash(hash, 2);
    }
    //change
    return true;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
fuzzyBFF<ItemType, FingerprintType, HashFamily>::~fuzzyBFF() {
    // Destructor logic (if any cleanup is needed)
    // Leave empty if no special cleanup is required
}

#endif // FUZZY_BFF_H