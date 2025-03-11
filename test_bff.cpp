#include <iostream>
#include <vector>
#include <cmath>
#include "bff.h" // Include the header file
#include "timer.h"
#include <random>
#include <unordered_set>


::std::vector<::std::uint64_t> GenerateRandom64Fast(::std::size_t count,
    uint64_t start) {
::std::vector<::std::uint64_t> result(count);
uint64_t index = start;
auto genrand = [&index]() {
// mix64
uint64_t x = index++;
x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9L;
x = (x ^ (x >> 27)) * 0x94d049bb133111ebL;
x = x ^ (x >> 31);
return x;
};
::std::generate(result.begin(), result.end(), ::std::ref(genrand));
return result;
}

using namespace std;

int main() {
    // Define the size of the filter
    size_t size = 10000;
    size_t testsize = 1000000;
    size_t actual_testsize = testsize; // will remove elements that are in both sets (data and notinset)

    // Create an object of the fuzzyBFF class
    BFF<uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift> myFilter(size);

    // Print some of the initialized values (optional)
    myFilter.printInfo();

    // Create set of random keys not in the set
    vector<uint64_t> notinset(testsize);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    // Prepare data to populate
    std::unordered_set<uint64_t> keys;
    while(keys.size() != size){
        keys.insert(dis(gen));
    }
    std::vector<uint64_t> data(keys.begin(), keys.end());
    // vector<uint64_t> data = GenerateRandom64Fast(size, rand()) ;


    // Call the populate function
    Timer popTime;
    bool success = myFilter.populate(data, data.size());
    double time = popTime.Stop();
    cout << "Populate time: " << time << " microseconds total" << endl;
    cout << "Populate time:" << time/size << " microseconds per item" << endl;

    // Get the pointer to the filter
    uint32_t* filter = myFilter.getFilter();

    int fncount = 0;
    int fpcount = 0;
    
    // Check membership of all the keys
    for (size_t i = 0; i < size; ++i) {
        bool isMember = myFilter.membership(data[i]);
        // False negative
        if(!isMember){
            cout << "False negative: " << data[i] << endl;
            fncount++;
  
        }
    }

    // Generate random keys to test for false positives
    for (int i = 0; i < testsize; ++i) {
        notinset[i] = dis(gen);
    }
    
    int fp = 0;
    // Check membership of keys not in set
    for (uint64_t i = 0; i < testsize; ++i) {
        bool isMember = myFilter.membership(notinset[i]);
        // False positive?
        if(isMember){
            // Check if actually a false positive
            fp = 1;
            for (size_t j = 0; j < size; ++j) {
                if(notinset[i] == data[j]){
                    // no false positive
                    fp = 0;
                    actual_testsize--;
                    break;
                }
            }
            if(fp == 1){
                cout << "False positive: " << notinset[i] << endl;
                fpcount++;
            }
            
        }
    }

    // fp and fn fraction
    cout << "False negative fraction: " << fncount << "/" << size << endl;
    cout << "False positive fraction: " << fpcount << "/" << actual_testsize << endl;

    return 0;
}
