#ifndef fuzzyBFF_H
#define fuzzyBFF_H

// Include necessary standard libraries
#include <stdlib.h>
#include <vector>
#include "hashfunction.h"
#include <unordered_set>
#include "lsh.h"
#include "examplelsh.h"

using namespace std;

template <typename InputType, typename ItemType, typename FingerprintType, typename HashFamily, typename LSHType>
class fuzzyBFF {
public:
    // Constructor
    fuzzyBFF() {

        //Check if LSHType InputType and ItemType are compatible with fuzzyBFF InputType and ItemType
        static_assert(std::is_base_of<LSH<InputType, ItemType>, LSHType>::value, "LSHType must be derived from LSH<InputType, ItemType>");

        this->size = 0;
        this->segmentLength = 0;
        this->arrayLength = 0;
        this->segmentCount = 0;
        this->hashfunction = new HashFamily();
        this->filter = nullptr;
    }

    // Destructor
    ~fuzzyBFF();

    //////////////////////////
    // Public member functions
    //////////////////////////

    // Populate with data in vector data
    bool populate(const vector<InputType>& data, size_t length) {
        return populate(data.data(), length);
    }

    // Populate with data in array data
    bool populate(const InputType* data, size_t length);

    // Get pointer to filter
    FingerprintType* getFilter() {
        return this->filter;
    }

    // See https://github.com/FastFilter/fastfilter_cpp
    inline __attribute__((always_inline)) size_t getHashFromHash(uint64_t hash, int index) {
        __uint128_t x = (__uint128_t)hash * (__uint128_t)(this->segmentCountLength);
        uint64_t h = (uint64_t)(x >> 64);
        h += index * this->segmentLength;
        // keep the lower 36 bits
        uint64_t hh = hash & ((1UL << 36) - 1);
        // index 0: right shift by 36; index 1: right shift by 18; index 2: no shift
        h ^= (size_t)((hh >> (36 - 18 * index)) & (this->segmentLength - 1));
        return h;
    }

    bool membership(InputType& item);

    //get hashfunction for debugging
    HashFamily* getHashFunction() {
        return this->hashfunction;
    }

    void printInfo() {
        cout << "Filter details:" << endl;
        cout << "Size:\t" << this->size << endl;
        cout << "Segment Length:\t" << this->segmentLength << endl;
        cout << "Segment Count:\t" << this->segmentCount << endl;
        cout << "Array Length:\t" << this->arrayLength << endl;
    }

    // Public member variables
    // TODO: make private after testing?
    size_t size;
    size_t segmentLength;
    size_t arrayLength;
    size_t segmentCount;
    size_t segmentLengthMask;
    size_t segmentCountLength;
    HashFamily* hashfunction;

    // Filter array
    FingerprintType* filter;

private:
//TODO: need to initialize sth for some cases? 
    LSHType lsh;

    // Initialize the filter
    inline __attribute__((always_inline)) void initFilter() {
        // TODO: Have different values than their implementation 
        // Calculate all necessary parameters to setup filter
        this->size = size;
        this->segmentLength = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // TODO: check why/if needed
        if (this->segmentLength > (1 << 18)) {
            this->segmentLength = (1 << 18);
        }
        // Filter needs to be bigger than size of input set
        double factor = fmax(1.125, 0.875 + 0.25 * log(1000000) / log(size));
        this->arrayLength = factor * size;

        // We need to fit an integer number of segments in the filter
        this->segmentCount = ((this->arrayLength + this->segmentLength - 1) / this->segmentLength);

        // For very small set sizes
        if(this->segmentCount < 3){
            this->segmentCount = 3;
        }

        // Size of the logical filter array
        this->arrayLength = (this->segmentCount) * this->segmentLength;
        
        // Parameters used for getHashFromHash function
        this->segmentLengthMask = this->segmentLength - 1;
        this->segmentCountLength = (this->segmentCount-2) * this->segmentLength;

        // Allocate memory for filter
        this->filter = new FingerprintType[this->arrayLength]();
        std::fill_n(this->filter, this->arrayLength, 0);

        // Initialize hash function
        this->hashfunction = new HashFamily();
    }
};

template <typename InputType, typename ItemType, typename FingerprintType, typename HashFamily, typename LSHType>
fuzzyBFF<InputType, ItemType, FingerprintType, HashFamily, LSHType>::~fuzzyBFF() {
    delete[] this->filter;
    delete this->hashfunction;
}

template <typename InputType, typename ItemType, typename FingerprintType, typename HashFamily, typename LSHType>
bool fuzzyBFF<InputType, ItemType, FingerprintType, HashFamily, LSHType>::populate(const InputType* data, size_t length) {
    // Map keys with LSH
    // Get rid of duplicates
    // TODO: consider efficiency of this step with unordered set

    std::unordered_set<InputType> keys;
    for (size_t i = 0; i < length; i++) {
        std::vector<ItemType> lsh_keys = lsh.hash_values(data[i]);
        for (size_t j = 0; j < lsh_keys.size(); j++) {
            keys.insert(lsh_keys[j]);
        }
    }
    std::vector<ItemType> data_new(keys.begin(), keys.end());
    
    this->size = data_new.size();
    length = data_new.size();

    // Initialize filter
    initFilter();  

    // Initialize arrays for array C
    // First mapping of values to their positions in the filter
    uint64_t* arrayC_hash = new uint64_t[arrayLength];
    uint64_t* arrayC_count = new uint64_t[arrayLength];

    // Initialize arrays for stack Q
    // Stack of singletons
    size_t* stackQ = new size_t[arrayLength];

    // Initialize counters for stack Q
    size_t stackQ_pos = 0;

    // Initialize arrays for stack P
    // Stack of hash/index pairs in order of detection
    size_t* stackP_index = new size_t[arrayLength];
    uint8_t* stackP_hi = new uint8_t[arrayLength];

    // Initialize counters for stack P
    size_t stackP_pos = 0;

    // Initialize xor value
    FingerprintType xor2 = 0;

    
    while (true) {
        memset(arrayC_hash, 0, sizeof(uint64_t[arrayLength]));
        memset(arrayC_count, 0, sizeof(size_t[arrayLength]));

        // Scan through keys in set and add hashes of x 
        // to sets in array C corresponding to h_0(x), h_1(x), h_2(x). 
        // Increase counter accordingly.
        for (size_t i = 0; i < length; i++) {
            uint64_t key = data_new[i];
            uint64_t hash = (*hashfunction)(key);
            for (int hi = 0; hi < 3; hi++) {
                size_t index = getHashFromHash(hash, hi);
                arrayC_hash[index] ^= hash;
                arrayC_count[index]++;
            }
        }

        // Scan through array C and add singletons to stack Q
        for (size_t i = 0; i < arrayLength; i++) {
            if (arrayC_count[i] == 1) {
                stackQ[stackQ_pos++] = i;
            }
        }
        

        // Go through stack Q and add singletons to stack P
        while (stackQ_pos > 0) {
            stackQ_pos--;

            // Pop location from stack Q
            size_t index = stackQ[stackQ_pos];

            // Check if we still have an element at this location
            if (arrayC_count[index] == 1) {
                // Get the hash corresponding to the singleton at position index
                uint64_t hash = arrayC_hash[index];

                // Append location to stack P
                stackP_index[stackP_pos] = index;

                // Decrement counters of the three locations in array C
                for (int hi = 0; hi < 3; hi++) {
                    size_t index3 = getHashFromHash(hash, hi);
                    // Check if we are at the original location
                    // In this case we store the hi value for later use
                    if (index3 == index) {
                        stackP_hi[stackP_pos] = hi;
                        continue; 
                    }
                    // Check if we have a new singleton
                    else if (arrayC_count[index3] == 2) {
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
        if (stackP_pos == size) {
            printf("Construction successful\n");
            break;
        } else {
            printf("Construction failed, retrying...\n");
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
    while (stackP_pos > 0) {
        stackP_pos--;

        // Pop location from stack P
        size_t index = stackP_index[stackP_pos];

        // Get the hash corresponding to the location
        uint64_t hash = arrayC_hash[index];

        // Get the hi value corresponding to the location
        int hi_found = stackP_hi[stackP_pos];

        // Calculate xor value
        xor2 = (FingerprintType)hash;

        size_t index3 = 0;
        for (int hi = 0; hi < 3; hi++) {
            if (hi != hi_found) {
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

template <typename InputType, typename ItemType, typename FingerprintType, typename HashFamily, typename LSHType>
bool fuzzyBFF<InputType, ItemType, FingerprintType, HashFamily, LSHType>::membership(InputType& item) {

    ItemType lsh_item;
    uint64_t hash;
    FingerprintType xor2 ;

    std::vector<ItemType> lsh_keys = lsh.hash_values(item);
    for(int j = 0; j < lsh_keys.size(); j++){
        lsh_item = lsh_keys[j];
        printf("lsh_item: %llu\n", lsh_item);
        hash = (*hashfunction)(lsh_item);
        xor2 = (FingerprintType)hash;

        // TODO: want to make it faster? inline the function call and combine for all three hi values
        for (int hi = 0; hi < 3; hi++) {
            size_t h = getHashFromHash(hash, hi);
            xor2 ^= filter[h];
            printf("xor2: %u\n", xor2);
        }
        if(xor2 == 0){
            return true;
        }
    }
    return false;
    
}

#endif // FUZZY_BFF_H
