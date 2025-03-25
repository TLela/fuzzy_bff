#ifndef BFFwrapping_H
#define BFFwrapping_H


// Include necessary standard libraries
#include <stdlib.h>
#include <vector>
#include "hashfunction.h"
#include <fstream>


using namespace std;

template <typename ItemType, typename FingerprintType, typename HashFamily>
class BFFwrapping {
public:
    // Constructor
    BFFwrapping(const size_t size){
        // TODO: Have different values than their implementation (they add the 2 segments not always addiitonally but within the additional array size already considered I think...)
        // Calculate all necessary parameters to setup filter
        this->size = size;
        this->segmentLength = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // The current implementation hardcodes a 18-bit limit to
        // to the segment length as stated in the original implementation.
        if (this->segmentLength > (1 << 18)) {
            this->segmentLength = (1 << 18);
        }
        // Filter needs to be bigger than size of input set
        double factor = fmax(1.125, 0.875 + 0.25 * log(1000000) / log(size));
        this->arrayLength = factor * size;

        // We need to fit an integer number of segments in the filter
        this->segmentCount = ((this->arrayLength + this->segmentLength - 1) / this->segmentLength);

        // For very small set sizes
        if(this->segmentCount < 1){
            this->segmentCount = 1;
        }

        // Size of the logical filter array
        // We wrap around the last two segments to ensure uniform randomness of hash positions of elements
        // This is done by adding two additional segments at the end of the filter - these correspond to the first two segments
        this->arrayLength = (this->segmentCount + 2) * this->segmentLength;
        
        // Parameters used for getHashFromHash function
        this->segmentLengthMask = this->segmentLength - 1;
        this->segmentCountLength = (this->segmentCount)* this->segmentLength;

        // Allocate memory for filter
        this->filter = new FingerprintType[this->segmentCountLength]();
        std::fill_n(this->filter, this->segmentCountLength, 0);

        // Initialize hash function
        this->hashfunction = new HashFamily();
    }

    // Destructor
    ~BFFwrapping();

    // Public member functions
    // Populate with data in vector data
    bool populate(const vector<ItemType>& data, size_t length, int countRuns){
        return  populate(data.data(), length, countRuns);
    }

    // Populate with data in array data
    bool populate(const ItemType* data, size_t length, int countRuns);

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
        return h % this->segmentCountLength;
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
        cout << "Filter Length:\t" << this->segmentCountLength << endl;
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
BFFwrapping<ItemType, FingerprintType, HashFamily>::~BFFwrapping() {
    delete[] this->filter;
    delete this->hashfunction;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
bool BFFwrapping<ItemType, FingerprintType, HashFamily>::populate(const ItemType* data, size_t length, int countRuns){

    // Create file to write resulting singleton distribution
    string filename = "Results/BFFWRAPTEST" + to_string(countRuns) + ".txt";
    ofstream BFFwrappingfile(filename);

    // Check if the filter is big enough to hold the data
    if (length > this->size) {
        return false;
    }

    // Initialize arrays for array C
    // First mapping of values to their positions in the filter
    uint64_t *arrayC_hash = new uint64_t[segmentCountLength];
    uint64_t *arrayC_count = new uint64_t[segmentCountLength];

    // Initialize arrays for stack Q
    // Stack of singletons
    size_t *stackQ = new size_t[segmentCountLength];

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
        memset(arrayC_hash, 0, sizeof(uint64_t[segmentCountLength]));
        memset(arrayC_count, 0, sizeof(size_t[segmentCountLength]));

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

        //////////////////////////////////////////  

        // count singletons per segment
        size_t *singletonpersegment = new size_t[segmentCount];
        memset(singletonpersegment, 0, sizeof(size_t[segmentCount]));

        // count total mappings per segment
        size_t *countpersegment = new size_t[segmentCount];
        memset(countpersegment, 0, sizeof(size_t[segmentCount]));
        for(size_t i = 0; i < segmentCountLength; i++){
            if(arrayC_count[i] > 0){
                //find segment it belongs to
                size_t segment = i / segmentLength;
                countpersegment[segment]+=arrayC_count[i];
            }
        }
        //////////////////////////////////////////
        // Scan through array C and add singletons to stack Q
        for(size_t i = 0; i < segmentCountLength; i++){
            if(arrayC_count[i] == 1){
                stackQ[stackQ_pos++] = i;
                //////////////////////////////////////////  
                //find segment it belongs to
                size_t segment = i / segmentLength;
                singletonpersegment[segment]++;
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
                        //////////////////////////////////////////  
                        // // Uncomment this if you want to count the total number of singletons found over the
                        // // course of the whole construction.
                        // // Leave it out, if you want only the singletons found in the initial traversal of the data.
                       
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
        //////////////////////////////////////////  
        // count remaining mappings per segment
        size_t *rempersegment = new size_t[segmentCount];
        memset(rempersegment, 0, sizeof(size_t[segmentCount]));

        //print C_count
        for(size_t i = 0; i < segmentCountLength; i++){
            if(arrayC_count[i] >0){
                //find segment it belongs to
                size_t segment = i / segmentLength;
                rempersegment[segment]+=arrayC_count[i];
            }
        }

        BFFwrappingfile << "Segm,Singlt,Total,Ratio,Remaining" << endl;
        for (size_t i = 0; i < segmentCount; i++) {
            BFFwrappingfile<<
                i << "," <<
                singletonpersegment[i] << "," <<
                countpersegment[i] << "," <<
                (double)singletonpersegment[i] / (countpersegment[i]-rempersegment[i]) << "," <<
                rempersegment[i] << endl;
        }
        //////////////////////////////////////////
        

        // Check if construction was successful
        if (stackP_pos == size){
            printf("Construction successful\n");
            break;
        }
        else{
            printf("Construction failed, retrying...\n"); 
            //////////////////////////////////////////  
            return false;
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
bool BFFwrapping <ItemType, FingerprintType, HashFamily>::membership(ItemType &item) {

    uint64_t hash = (*hashfunction)(item);
    FingerprintType xor2 = (FingerprintType)hash;

    // TODO: want to make it faster? inline the function call and combine for all three hi values
    for(int hi = 0; hi < 3; hi++){
        size_t h = getHashFromHash(hash, hi);
        xor2 ^= filter[h];
    }
    
    return xor2 == 0;
}

#endif // BFFwrapping_H


#ifndef BFFTEST_H
#define BFFTEST_H



using namespace std;

template <typename ItemType, typename FingerprintType, typename HashFamily>
class BFFTEST {
public:
    // Constructor
    BFFTEST(const size_t size){
        // Calculate all necessary parameters to setup filter
        this->size = size;
        this->segmentLength = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // The current implementation hardcodes a 18-bit limit to
        // to the segment length as stated in the original implementation.
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
        this->segmentCountLength = (this->segmentCount - 2) * this->segmentLength;

        // Allocate memory for filter
        this->filter = new FingerprintType[this->arrayLength]();
        std::fill_n(this->filter, this->arrayLength, 0);

        // Initialize hash function
        this->hashfunction = new HashFamily();
    }

    // Destructor
    ~BFFTEST();

    // Public member functions
    // Populate with data in vector data
    bool populate(const vector<ItemType>& data, size_t length, int countRuns){
        return  populate(data.data(), length, countRuns);
    }

    // Populate with data in array data
    bool populate(const ItemType* data, size_t length, int countRuns);

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
BFFTEST<ItemType, FingerprintType, HashFamily>::~BFFTEST() {
    delete[] this->filter;
    delete this->hashfunction;
}

template <typename ItemType, typename FingerprintType, typename HashFamily>
bool BFFTEST<ItemType, FingerprintType, HashFamily>::populate(const ItemType* data, size_t length, int countRuns){

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
        //////////////////////////////////////////  

        //count singletons per segment
        size_t *singletonpersegment = new size_t[segmentCount];
        memset(singletonpersegment, 0, sizeof(size_t[segmentCount]));
        // count total mappings per segment
        size_t *countpersegment = new size_t[segmentCount];
        memset(countpersegment, 0, sizeof(size_t[segmentCount]));

        for(size_t i = 0; i < arrayLength; i++){
            if(arrayC_count[i] > 0){
                //find segment it belongs to
                size_t segment = i / segmentLength;
                countpersegment[segment]+=arrayC_count[i];
            }
        }
        //////////////////////////////////////////

        // Scan through array C and add singletons to stack Q
        for(size_t i = 0; i < arrayLength; i++){
            if(arrayC_count[i] == 1){
                stackQ[stackQ_pos++] = i;

                //////////////////////////////////////////  
                //find segment it belongs to
                size_t segment = i / segmentLength;
                singletonpersegment[segment]++;
                //////////////////////////////////////////
            }
        }

        // Create file to write resulting singleton distribution
        string filename = "Results/BFFTEST_0_" + to_string(countRuns) + ".txt";
        ofstream BFFTESTfile(filename);

        BFFTESTfile << "Segm,Singlt,Total,Ratio,Remaining" << endl;
        for (size_t i = 0; i < segmentCount; i++) {
            BFFTESTfile<<
                i << "," <<
                singletonpersegment[i] << "," <<
                countpersegment[i] << "," <<
                (double)singletonpersegment[i] / countpersegment[i] <<  endl;
        }
        BFFTESTfile.close();

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
                        arrayC_count[index3] = 0;
                        continue; 
                    }
                    // Check if we have a new singleton
                    else if(arrayC_count[index3] == 2){
                        stackQ[stackQ_pos++] = index3;

                        //////////////////////////////////////////  
                        // Uncomment this if you want to count the total number of singletons found over the
                        // course of the whole construction.
                        // Leave it out, if you want only the singletons found in the initial traversal of the data.
                       
                        size_t segment = index3 / segmentLength;
                        singletonpersegment[segment]++;
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
            
            for(int p = 1; p < 5; p++){
                if(stackP_pos == (size_t)(p * size/4)){
                    //printf("Current size of stackP_pos: %zu\n", stackP_pos);
                    // Create file to write resulting singleton distribution
                    string filename = "Results/BFFTEST_" + to_string(p) + "_" + to_string(countRuns) + ".txt";
                    ofstream BFFTESTfile(filename);

                    BFFTESTfile << "Segm,Singlt,Total,Ratio,Remaining" << endl;
                    for (size_t i = 0; i < segmentCount; i++) {
                        BFFTESTfile<<
                            i << "," <<
                            singletonpersegment[i] << "," <<
                            countpersegment[i] << "," <<
                            (double)singletonpersegment[i] / countpersegment[i] <<  endl;
                    }
                    BFFTESTfile.close();
                }
            }            
            
        }

        //////////////////////////////////////////  
        

        //////////////////////////////////////////

        // Check if construction was successful
        if (stackP_pos == size){
            printf("Construction successful\n");
            break;
        }
        else{
            printf("Construction failed, retrying...\n");
            //////////////////////////////////////////  
            return false;
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
bool BFFTEST <ItemType, FingerprintType, HashFamily>::membership(ItemType &item) {

    uint64_t hash = (*hashfunction)(item);
    FingerprintType xor2 = (FingerprintType)hash;

    // TODO: want to make it faster? inline the function call and combine for all three hi values
    for(int hi = 0; hi < 3; hi++){
        size_t h = getHashFromHash(hash, hi);
        xor2 ^= filter[h];
    }
    
    return xor2 == 0;
}

#endif // BFFTEST_H
