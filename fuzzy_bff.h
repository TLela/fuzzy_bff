#ifndef fuzzyBFF_H
#define fuzzyBFF_H

// Include necessary standard libraries
#include <stdlib.h>
#include <vector>
#include "hashfunction.h"
#include <unordered_set>


using namespace std;

// TODO - implement LSH class and instances
uint64_t lsh_hash(uint64_t key){
    return key;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
class fuzzyBFF {
public:
    // Constructor
    fuzzyBFF(const size_t size){
        // TODO: Have different values than their implementation (they add the 2 segments not always addiitonally but within the additional array size already considered I think...)
        // Calculate all necessary parameters to setup filter
        this->size = size;
        this->segmentLength = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // Filter needs to be bigger than size of input set
        double factor = fmax(1.125, 0.875 + 0.25 * log(1000000) / log(size));
        printf("factor: %f\n", factor);
        this->filterLength = factor * size;
        printf("filterLength: %lu\n", this->filterLength);

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
        __uint128_t x = (__uint128_t)hash * (__uint128_t)(this->filterLength);
        uint64_t h = (uint64_t)(x >> 64);
        h += index * this->segmentLength;
        // keep the lower 36 bits
        uint64_t hh = hash & ((1UL << 36) - 1);
        // index 0: right shift by 36; index 1: right shift by 18; index 2: no shift
        h ^= (size_t)((hh >> (36 - 18 * index)) & (this->segmentLength - 1));
        return h % (this->filterLength);
    }

    bool membership(ItemType &item);

    // Public member variables
    // TODO: make private after testing?
    size_t size;
    size_t segmentLength;
    size_t arrayLength;
    size_t filterLength;
    size_t segmentCount;
    HashFamily *hashfunction;

    // Filter array
    FingerprintType* filter;

};

template <typename ItemType, typename FingerprintType, typename HashFamily>
fuzzyBFF<ItemType, FingerprintType, HashFamily>::~fuzzyBFF() {
    delete[] this->filter;
    delete this->hashfunction;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
bool fuzzyBFF<ItemType, FingerprintType, HashFamily>::populate(const ItemType* data, size_t length){

    // Check if the filter is big enough to hold the data
    if (length > this->size) {
        return false;
    }

    // Map keys with LSH
    // Get rid of duplicates
    // TODO: consider efficiency of this step with unordered set
    std::unordered_set<uint64_t> keys;
    for (size_t i = 0; i < length; i++){
        keys.insert(lsh_hash(data[i]));
    }
    std::vector<uint64_t> data_new(keys.begin(), keys.end());
    
    size = data_new.size();
    length = size;

    // Initialize arrays for array C
    // First mapping of values to their positions in the filter
    uint64_t *arrayC_hash = new uint64_t[filterLength];
    uint64_t *arrayC_count = new uint64_t[filterLength];

    // Initialize arrays for stack Q
    // Stack of singletons
    size_t *stackQ = new size_t[filterLength];

    // Initialize counters for stack Q
    size_t stackQ_pos = 0;

    // Initialize arrays for stack P
    // Stack of hash/index pairs in order of detection
    size_t *stackP_index = new size_t[filterLength];
    uint8_t *stackP_hi = new uint8_t[filterLength];

    // Initialize counters for stack P
    size_t stackP_pos = 0;

    // Initialize xor value
    FingerprintType xor2 = 0;

    
    while(true){
        memset(arrayC_hash, 0, sizeof(uint64_t[filterLength]));
        memset(arrayC_count, 0, sizeof(size_t[filterLength]));

        // Scan through keys in set and add hashes of x 
        // to sets in array C corresponding to h_0(x), h_1(x), h_2(x). 
        // Increase counter accordingly.
        for(size_t i = 0; i < length; i++){
            uint64_t key = data_new[i];
            uint64_t hash = (*hashfunction)(key);
            for (int hi = 0; hi < 3; hi++){
                size_t index = getHashFromHash(hash, hi);
                arrayC_hash[index] ^= hash;
                arrayC_count[index]++;
            }
            // printf("arrayC_count: ");
            // for(size_t i = 0; i < filterLength; i++){
            //     printf("%llu ", arrayC_count[i]);
            // }
            // printf("  For hash value: %llu\n", hash);
            // printf("\n");
        }

        // Scan through array C and add singletons to stack Q
        for(size_t i = 0; i < filterLength; i++){
            if(arrayC_count[i] == 1){
                stackQ[stackQ_pos++] = i;
            }
        }
        

        // Go through stack Q and add singletons to stack P
        while(stackQ_pos > 0){
            stackQ_pos--;

            // Pop location from stack Q
            size_t index = stackQ[stackQ_pos];


            // Check if we still have an element at this location
            if(arrayC_count[index]==1){
                // Get the hash corresponding to the singleton at position index
                uint64_t hash = arrayC_hash[index];

                // Append location to stack P
                stackP_index[stackP_pos] = index;

                // Decrement counters of the three locations in array C
                for(int hi = 0; hi < 3; hi++){
                    size_t index3 = getHashFromHash(hash, hi);
                    // Check if we are at the original location
                    // In this case we store the hi value for later use
                    if(index3 == index){
                        stackP_hi[stackP_pos] = hi;
                        continue; 
                    }
                    // Check if we have a new singleton
                    else if(arrayC_count[index3] == 2){
                        stackQ[stackQ_pos++] = index3;
                    }
                    // Decrement counter
                    // update hash
                    arrayC_hash[index3] ^= hash;
                    arrayC_count[index3]--;
                }
                // Increase stack P position
                stackP_pos++;
            }
            
        }

        // Check if construction was successful
        if (stackP_pos == size){
            printf("Construction successful\n");
            break;
        }

        // If not, generate new hash functions
        delete hashfunction;
        hashfunction = new HashFamily();

        // Reset stack counters
        stackP_pos = 0;
        stackQ_pos = 0;
    }
    // Clean up
    delete[] arrayC_count;
    delete[] stackQ;

    // Populate filter
    while(stackP_pos > 0){
        stackP_pos--;

        // Pop location from stack P
        size_t index = stackP_index[stackP_pos];

        // Get the hash corresponding to the location
        uint64_t hash = arrayC_hash[index];

        // Caclulate xor value
        xor2 = (FingerprintType)hash;
        size_t index3 = 0;
        for(int hi = 0; hi < 3; hi++){
            if(hi != index){
                // xor the filter value at the location h_hi(x)
                index3 = getHashFromHash(hash, hi);
                xor2 ^= filter[index3];
            }   
        }
        // Update filter 
        filter[index] = xor2;
    }
    delete[] stackP_index;
    delete[] stackP_hi;
    delete[] arrayC_hash;

    return true;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
bool fuzzyBFF <ItemType, FingerprintType, HashFamily>::membership(ItemType &item) {

    uint64_t hash = (*hashfunction)(item);
    FingerprintType xor2 = (FingerprintType)hash;

    // TODO: want to make it faster? inline the function call and combine for all three hi values
    for(int hi = 0; hi < 3; hi++){
        size_t h = getHashFromHash(hash, hi);
        xor2 ^= filter[h];
    }
    
    return xor2 == 0;
}

#endif // fuzzyBFF_H
