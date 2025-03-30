#include <iostream>
#include <vector>
#include <cmath>
#include "../bff.h" // Include the header file
#include "../Utils/timer.h"
#include <random>
#include <unordered_set>


using namespace std;

int main() {
    // Define the size of the filter
    size_t size = 1000000;
    size_t testsize = 1000000;

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
    
    // Create and populate the filter
    Timer popTime;
    BFF<uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift> myFilter(size);
    bool success = myFilter.populate(data, data.size());
    double time = popTime.Stop();
    
    cout << endl;
    cout << "//////////Timing results://////////" << endl;
    cout << "Populate time: " << time << " microseconds total" << endl;
    cout << "Populate time:" << time/size << " microseconds per item" << endl;
    cout << endl;
    // Print some of the initialized values (optional)
    myFilter.printInfo();
    
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
    int fp = 0;
    size_t count = 0;
    uint64_t candidate = 0;
    while(count < testsize){
        candidate = dis(gen);
        // Check if candidate is in the set
        if(keys.find(candidate) == keys.end()){
            notinset[count] = candidate;
            count++;
        }
    }
    // Check membership of keys not in set
    for (size_t i = 0; i < testsize; ++i) {
        bool isMember = myFilter.membership(notinset[i]);
        // False positive
        if(isMember){
            cout << "False positive: " << notinset[i] << endl;
            fpcount++;
        }
    }
    // fp and fn fraction
    cout << endl;
    cout << "//////////Query results:///////////" << endl;
    cout << "False negative fraction: " << fncount << "/" << size << endl;
    cout << "False positive fraction: " << fpcount << "/" << testsize << endl;

    return 0;
}
