#include <iostream>
#include <vector>
#include <cmath>
#include "fuzzy_bff.h" // Include the header file
#include "examplelsh.h"
#include "examplelsh2.h"
#include "lsh_bitsample.h"
#include "timer.h"

using namespace std;

int main() {
    // Define the size of the filter and the size of the test set
    size_t size = 100000;
    size_t testsize = 1000;

    // Create an object of the fuzzyBFF class
    fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift, BitSampleLSH> myFilter;

    // Generate random keys to populate the filter
    vector<uint64_t> data(size);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    
    BitSampleLSH lsh;

    // Generate keys close to the above set to check false negatives
    // flip r_1 fraction of bits
    vector<uint64_t> closedata(testsize);
    std::uniform_int_distribution<int> close(0, 63);
    std::unordered_set<int> flipped_bits; 
    double r_1 = lsh.r_1;
    for(int i = 0; i < testsize; i++){
        while(flipped_bits.size() < r_1 * 64 - 1){
            flipped_bits.insert(close(gen));
        }
        closedata[i] = data[i];
        for (int flipped_bit : flipped_bits) {
            closedata[i] ^= (1ULL << flipped_bit);
        }  
        flipped_bits.clear();   
    }
    

    // Generate keys far from the above set to check false positives
    // flip r_2+1 fraction of bits
    vector<uint64_t> fardata(testsize);
    std::uniform_int_distribution<int> far(0, 63);
    std::unordered_set<int> flipped_bits2;
    double r_2 = lsh.r_2;
    for(int i = 0; i < testsize; i++){
        while(flipped_bits2.size() < r_2 * 64 + 1){
            flipped_bits2.insert(far(gen));
        }
        fardata[i] = data[i];
        for (int flipped_bit : flipped_bits2) {
            fardata[i] ^= (1ULL << flipped_bit);
        }  
        flipped_bits2.clear();   
    }

    // Call the populate function
    bool success = myFilter.populate(data, data.size());
    
    // Print some of the initialized values
    myFilter.printInfo();

    int fncount = 0;
    int fpcount = 0;

    // Check for false negatives
    for (int i = 0; i < testsize; i++) {
        if (myFilter.membership(closedata[i]) == false) {
            fncount++;
        }
    }
    // Check for false positives
    for (int i = 0; i < testsize; i++) {
        if (myFilter.membership(fardata[i]) == true) {
            fpcount++;
        }
    }
   
    // fp and fn fraction
    cout << "False negatives: " << fncount <<  endl;
    cout << "False positives: " << fpcount << endl;

    return 0;
}
