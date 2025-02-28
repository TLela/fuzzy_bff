#include <iostream>
#include <vector>
#include <cmath>
#include "fuzzy_bff.h" // Include the header file
#include "examplelsh.h"
#include "examplelsh2.h"
#include "timer.h"

using namespace std;

int main() {
    // Define the size of the filter
    size_t size = 100000;

    // Create an object of the fuzzyBFF class
    fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift, ExampleLSH> myFilter;

    // Prepare data to populate
    // random elements
    vector<uint64_t> data(size);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0,2*size);

    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }

    // Call the populate function
    bool success = myFilter.populate(data, data.size());

    // Print some of the initialized values (optional)
    cout << "Filter details:" << endl;
    cout << "Size: " << myFilter.size << endl;
    cout << "Segment Length: " << myFilter.segmentLength << endl;
    cout << "Segment Count: " << myFilter.segmentCount << endl;
    cout << "Array Length: " << myFilter.arrayLength << endl;

    // Get the pointer to the filter
    uint32_t* filter = myFilter.getFilter();

    int fncount = 0;
    int fpcount = 0;
    int notinsetsize = size;

    ExampleLSH lsh;
    // Check membership of all the keys
    for (size_t i = 0; i < size; ++i) {
        uint64_t newkey = data[i];
        bool isMember = myFilter.membership(newkey);
        // False negative
        if (!isMember) {
            //cout << "False negative: " << data[i] << endl;
            fncount++;
        }
    }

    // Check membership of notinsetsize keys >= size
    for (uint64_t i = size; i < size + notinsetsize; ++i) {
        bool isMember = myFilter.membership(i);
        // False positive
        if (isMember) {
            //cout << "False positive: " << i << endl;
            fpcount++;
        }
    }

    // fp and fn fraction
    cout << "False negatives: " << fncount <<  endl;
    cout << "False positives: " << fpcount << endl;

    return 0;
}
