#include <iostream>
#include <vector>
#include <cmath>
#include "fuzzy_bff.h"
#include "lsh_bitsample.h"
#include "timer.h"

using namespace std;

int main() {
    // Define the size of the filter and the size of the test set
    size_t maxsize = 1000000;
    size_t maxtestsize = 1000000;

    // Create an object of the fuzzyBFF class
    fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift, BitSampleLSH> myFilter;

    BitSampleLSH lsh = myFilter.lsh;

    // Generate random keys to populate the filter
    cout << "Generating random keys..." << endl;
    vector<uint64_t> data(maxsize);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    for (int i = 0; i < maxsize; ++i) {
        data[i] = dis(gen);
    }


    // Generate keys close to the above set to check false negatives
    // flip r_1 fraction of bits
    cout << "Generating close keys..." << endl;
    vector<uint64_t> closedata(maxtestsize);
    std::uniform_int_distribution<int> close(0, 63);
    std::unordered_set<int> flipped_bits; 
    double r_1 = lsh.r_1;
    for(int i = 0; i < maxtestsize; i++){
        while(flipped_bits.size() < std::floor(r_1 * 64)){
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
    cout << "Generating far keys..." << endl;
    vector<uint64_t> fardata(maxtestsize);
    std::uniform_int_distribution<int> far(0, 63);
    std::unordered_set<int> flipped_bits2;
    double r_2 = lsh.r_2;
    for(int i = 0; i < maxtestsize; i++){
        while(flipped_bits2.size() < std::ceil(r_2 * 64)){
            flipped_bits2.insert(far(gen));
        }
        fardata[i] = data[i];
        for (int flipped_bit : flipped_bits2) {
            fardata[i] ^= (1ULL << flipped_bit);
        }  
        flipped_bits2.clear();   
    }

    // repeat the test for different sizes
    cout << "Starting tests..." << endl;
    size_t size;
    size_t testsize;
    std::vector <uint64_t> sizes_list = {1000, 10000, 100000, 1000000};
    std::vector <uint64_t> testsizes_list = {1000, 10000, 10000, 10000};
    for(int i = 0; i < testsizes_list.size(); i++){
        size = testsizes_list[i];
        testsize = sizes_list[i];

        // Call the populate function
        Timer popTime;
        bool success = myFilter.populate(data, size);
        double time = popTime.Stop();
        cout << "Populate time: " << time << " microseconds total" << endl;
        cout << "Populate time:" << time/size << " microseconds per item" << endl;
        cout << "Populate time:" << time/myFilter.size << " microseconds per actually inserted item" << endl;

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
        cout << "False negatives: " << fncount << "/" << testsize << endl;
        cout << "False positives: " << fpcount << "/" << testsize << endl;

        myFilter.deleteFilter();
    }
        
    return 0;
}
