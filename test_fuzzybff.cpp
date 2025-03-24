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
    size_t testsize = 10000;

    cout << "Initial Set Size: " << size << endl;
    cout << "Test Set Size: " << testsize << endl; 
    
    // Create an object of the fuzzyBFF class
    fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::SimpleMixSplit, BitSampleLSH> myFilter;
    BitSampleLSH lsh = myFilter.lsh;

    // print expected total fp/fn 
    //cout << "Expected false negatives: " << testsize * (1-(lsh.p_1 + pow(2,-64))) << endl;
    //cout << "Expected false positives: " << testsize * (size * lsh.p_2 + pow(2,-64)) << endl;

    // Generate random keys to populate the filter
    vector<uint64_t> data(size);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    
    // Generate keys close to the above set to check false negatives
    // flip r_1 fraction of bits
    vector<uint64_t> closedata(testsize);
    std::uniform_int_distribution<int> close(0, 63);
    std::unordered_set<int> flipped_bits; 
    double r_1 = lsh.r_1;
    for(int i = 0; i < testsize; i++){
        while(flipped_bits.size() < r_1){ 
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
    // vector<uint64_t> fardata(testsize);
    // std::uniform_int_distribution<int> far(0, 63);
    // std::unordered_set<int> flipped_bits2;
    // double r_2 = lsh.r_2;
    // for(int i = 0; i < testsize; i++){
    //     while(flipped_bits2.size() < r_2 + 1){
    //         flipped_bits2.insert(far(gen));
    //     }
    //     fardata[i] = data[i];
    //     for (int flipped_bit : flipped_bits2) {
    //         fardata[i] ^= (1ULL << flipped_bit);
    //     }  
    //     flipped_bits2.clear();   
    // }
    // Generate random keys and check that they differ at least in ?r_2+1? bits for all data points
    vector<uint64_t> fardata(testsize);
    for(int i = 0; i < testsize; i++){
        fardata[i] = dis(gen);
        bool found = false;
        while(found!=true){
            int count = 0;
            for(int j = 0; j < size; j++){
                int diff = __builtin_popcountll(fardata[i] ^ data[j]);
                if(diff < lsh.r_2 + 1){
                    fardata[i] = dis(gen);
                    //cout << "Count: " << count << endl;
                    break;
                }
                count++;
            }
            if(count == size){
                found = true;
                //cout << "Found far data point: " << i << endl;
            }
        }
    }
    cout << "Data generation completed." << endl;

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
            
            // cout << "False negative: " << closedata[i] << endl; 
            // // print fn in bit representation and data as well
            // cout << "FN:" << bitset<64>(closedata[i]) << endl;
            // cout << "Da:" << bitset<64>(data[i]) << endl;
            // // print bit representation of bitmasks
            // for (int j = 0; j < lsh.or_op; j++) {
            //     cout << j << " " ;
            //     for (int k = 63; k >= 0; k--) {
            //         cout << lsh.bitmasks[j][k];
            //     }
            //     cout << endl;
            // }
            // // print hash values for FN and Data
            // cout << "Hash values:" << endl;
            // vector<uint64_t> lsh_keys = lsh.hash_values(closedata[i]);
            // vector<uint64_t> lsh_keys2 = lsh.hash_values(data[i]);
            // for (int j = 0; j < lsh_keys.size(); j++) {
            //     cout << j << " " << bitset<64>(lsh_keys[j]) << endl;
            //     cout << j << " "  << bitset<64>(lsh_keys2[j]) << endl;
            // }

            // //save bits where fn and data differ in vector
            // vector<int> diffbits;
            // for (int j = 0; j < 64; j++) {
            //     if (((closedata[i] >> j) & 1) != ((data[i] >> j) & 1)) {
            //         diffbits.push_back(j);
            //     }
            // }
            // cout << "Diffbits: " << diffbits.size() << endl;
            // // check for bitmask that is equal to zero in all bits where fn and data differ
            // for (int j = 0; j < lsh.or_op; j++) {
            //     int count = 0;
            //     for (int k = 0; k < diffbits.size(); k++) {
            //         if (lsh.bitmasks[j][diffbits[k]] == 0) {
            //             count++;
            //         }
            //     }
            //     if (count == diffbits.size()) {
            //         cout << "Bitmask: " << j << " " << count << endl;
            //         for (int k = 63; k >= 0; k--) {
            //             cout << lsh.bitmasks[j][k];
            //         }
            //         cout << endl;
            //     }
            // }
            
        }
    }
    // Check for false positives
    for (int i = 0; i < testsize; i++) {
        if (myFilter.membership(fardata[i]) == true) {
            fpcount++;
            // if(fpcount++ < 0){
            //     cout << "False positive at position " << i << ": " << fardata[i] << endl; 
            //     //check which of the data points has the same fingerprint, and for which bitmask
            //     vector<uint64_t> lsh_keys = lsh.hash_values(fardata[i]);
            //     for (int j = 0; j < size; j++){
            //         vector<uint64_t> lsh_keys2 = lsh.hash_values(data[j]);
            //         for (int k = 0; k < lsh_keys.size(); k++){
            //             if(lsh_keys[k] == lsh_keys2[k]){
            //                 // cout << "Same fingerprint for datapoint: " << j << endl;
            //                 cout << "Bitmask: " << k << endl;
            //                 // cout << "Hash value: " << lsh_keys[k] << endl;

            //                 // // everything in bits
            //                 // cout << bitset<64>(fardata[i]) << endl;
            //                 // cout << bitset<64>(data[j]) << endl;
            //                 // for (int m = 63; m >= 0; m--) {
            //                 //     cout << lsh.bitmasks[k][m];
            //                 // }
            //                 // cout << endl;
            //                 // cout << bitset<64>(lsh_keys[k]) << endl;
            //                 // cout << bitset<64>(lsh_keys2[k]) << endl;
            //             }  
            //         }
            //     }
            // }
        }
    }
   
    // fp and fn fraction
    cout << "False negatives: " << fncount << "/" << testsize << endl;
    cout << "False positives: " << fpcount << "/" << testsize << endl;

    return 0;
}
