#ifndef BFF_H
#define BFF_H

// Include necessary standard libraries
#include <stdlib.h>
#include <vector>
#include "hashfunction.h"


using namespace std;

template <typename ItemType, typename FingerprintType, typename HashFamily>
class BFF {
public:
    // Constructor
    BFF(const size_t size){
        // TODO: Have different values than their implementation
        // Calculate all necessary parameters to setup filter
        this->size = size;
        this->segmentLength = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // The current implementation hardcodes a 18-bit limit to
        // to the segment length as stated in the original implementation.
        if (this->segmentLength > (1 << 18)) {
            this->segmentLength = (1 << 18);
        }
        // Filter needs to be bigger than size of input set
        double factor = fmax(1.125, 0.875 -  + 0.25 * log(1000000) / log(size));
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
        this->segmentCountLength = (this->segmentCount - 2) * this->segmentLength;

        // Allocate memory for filter
        this->filter = new FingerprintType[this->arrayLength]();
        std::fill_n(this->filter, this->arrayLength, 0);

        // Initialize hash function
        this->hashfunction = new HashFamily();
    }

    // Destructor
    ~BFF();

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
        __uint128_t x = (__uint128_t)hash * (__uint128_t)(segmentCountLength);
        uint64_t h = (uint64_t)(x >> 64);
        h += index * this->segmentLength;
        // keep the lower 36 bits
        uint64_t hh = hash & ((1UL << 36) - 1);
        // index 0: right shift by 36; index 1: right shift by 18; index 2: no shift
        h ^= (size_t)((hh >> (36 - 18 * index)) & (segmentLengthMask));
        return h;
    }

    bool membership(ItemType &item);

    //get hashfunction for debugging
    HashFamily* getHashFunction(){
        return this->hashfunction;
    }

    void printInfo() {
        cout << "Filter details:" << endl;
        cout << "Size:\t" << this->size << endl;
        cout << "Segment Length:\t" << this->segmentLength << endl;
        cout << "Segment Count:\t" << this->segmentCount << endl;
        cout << "Filter Length:\t" << this->arrayLength << endl;
    }

    // Public member variables
    size_t size;
    size_t segmentLength;
    size_t arrayLength;
    size_t segmentCount;
    size_t segmentLengthMask;
    size_t segmentCountLength;
    HashFamily *hashfunction;

    // Filter array
    FingerprintType* filter;

};

template <typename ItemType, typename FingerprintType, typename HashFamily>
BFF<ItemType, FingerprintType, HashFamily>::~BFF() {
    delete[] this->filter;
    delete this->hashfunction;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
bool BFF<ItemType, FingerprintType, HashFamily>::populate(const ItemType* data, size_t length){

    // Check if the filter is big enough to hold the data
    if (length > this->size) {
        return false;
    }

    // Initialize arrays for array C
    // First mapping of values to their positions in the filter
    uint64_t *arrayC_hash = new uint64_t[arrayLength];
    uint64_t *arrayC_count = new uint64_t[arrayLength];

    // Initialize arrays for stack Q
    // Stack of singletons
    size_t *stackQ = new size_t[arrayLength];

    // Initialize counters for stack Q
    size_t stackQ_pos = 0;

    // Initialize arrays for stack P
    // Stack of hash/index pairs in order of detection
    size_t *stackP_index = new size_t[size];
    uint8_t *stackP_hi = new uint8_t[size];

    // Initialize counters for stack P
    size_t stackP_pos = 0;

    // Initialize xor value
    FingerprintType xor2 = 0;

    
    while(true){
        memset(arrayC_hash, 0, sizeof(uint64_t[arrayLength]));
        memset(arrayC_count, 0, sizeof(size_t[arrayLength]));

        // Scan through keys in set and add hashes of x 
        // to sets in array C corresponding to h_0(x), h_1(x), h_2(x). 
        // Increase counter accordingly.
        for(size_t i = 0; i < length; i++){
            uint64_t key = data[i];
            uint64_t hash = (*hashfunction)(key);
            for (int hi = 0; hi < 3; hi++){
                size_t index = getHashFromHash(hash, hi);
                arrayC_hash[index] ^= hash;
                arrayC_count[index]++;
            }
        }
        ////////////////////////////////////////// Debugging

        // count singletons per segment
        // size_t *singletonpersegment = new size_t[segmentCount];
        // memset(singletonpersegment, 0, sizeof(size_t[segmentCount]));
        // // count total mappings per segment
        // size_t *countpersegment = new size_t[segmentCount];
        // memset(countpersegment, 0, sizeof(size_t[segmentCount]));

        // for(size_t i = 0; i < arrayLength; i++){
        //     if(arrayC_count[i] > 0){
        //         //find segment it belongs to
        //         size_t segment = i / segmentLength;
        //         countpersegment[segment]+=arrayC_count[i];
        //     }
        // }
        //////////////////////////////////////////

        // Scan through array C and add singletons to stack Q
        for(size_t i = 0; i < arrayLength; i++){
            if(arrayC_count[i] == 1){
                stackQ[stackQ_pos++] = i;

                ////////////////////////////////////////// Debugging
                // //find segment it belongs to
                // size_t segment = i / segmentLength;
                // singletonpersegment[segment]++;
                //////////////////////////////////////////
            }
        }

        

        // Go through stack Q and add singletons to stack P
        while(stackQ_pos > 0){
            stackQ_pos--;

            // Pop location from stack Q
            size_t index = stackQ[stackQ_pos];
            // printf("index: %zu\n", index);

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
                        arrayC_count[index3] = 0;
                        continue; 
                    }
                    // Check if we have a new singleton
                    else if(arrayC_count[index3] == 2){
                        stackQ[stackQ_pos++] = index3;
                        // printf("New singleton found\n");
                        ////////////////////////////////////////// Debugging
                        // //find segment it belongs to
                        // size_t segment = index3 / segmentLength;
                        // singletonpersegment[segment]++;
                        //////////////////////////////////////////
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

        ////////////////////////////////////////// Debugging
        // count remaining mappings per segment
        // size_t *rempersegment = new size_t[segmentCount];
        // memset(rempersegment, 0, sizeof(size_t[segmentCount]));

        // //print C_count
        // for(size_t i = 0; i < arrayLength; i++){
        //     if(arrayC_count[i] >0){
        //         //find segment it belongs to
        //         size_t segment = i / segmentLength;
        //         rempersegment[segment]+=arrayC_count[i];
        //     }
        // }
        // printf("Segm | Singlt | Total  | Ratio     |  Remaining (total) \n");
        // printf("----------------------------------------------------------------------------\n");

        // for (size_t i = 0; i < segmentCount; i++) {
        //     printf("%-4zu | %-6zu | %-6zu | %-9.6f | %-18zu\n", 
        //         i, 
        //         singletonpersegment[i], 
        //         countpersegment[i], 
        //         (double)singletonpersegment[i] / countpersegment[i], 
        //         rempersegment[i]);
        // }
        //////////////////////////////////////////

        // Check if construction was successful
        if (stackP_pos == size){
            printf("Construction successful\n");
            break;
        }
        else{
            printf("Construction failed, retrying...\n");
            ////////////////////////////////////////// Debugging
            // exit(1);
            //////////////////////////////////////////
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

        // Get the hi value corresponding to the location
        int hi_found = stackP_hi[stackP_pos];

        // Caclulate xor value
        xor2 = (FingerprintType)hash;
        size_t index3 = 0;
        for(int hi = 0; hi < 3; hi++){
            if(hi != hi_found){
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
bool BFF <ItemType, FingerprintType, HashFamily>::membership(ItemType &item) {

    uint64_t hash = (*hashfunction)(item);
    FingerprintType xor2 = (FingerprintType)hash;

    // TODO: want to make it faster? inline the function call and combine for all three hi values
    for(int hi = 0; hi < 3; hi++){
        size_t h = getHashFromHash(hash, hi);
        xor2 ^= filter[h];
    }
    
    return xor2 == 0;
}

#endif // BFF_H
