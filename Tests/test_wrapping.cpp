#include <iostream>
#include <vector>
#include <cmath>
#include "../bff_wrapping.h" // Include the header file
#include "../Utils/timer.h"
#include <random>

// Compare the BFF and BFFwrapping classes.
// We populate the filters with random data and check membership of the data.
// Due to the wrapping the latter class will fail repeatedly. This phenomenon is described further in the writeup.
// This test will visualize the distribution of mapped values to the filter array for both classes.

using namespace std;
void runtest(size_t size, int countRuns);

int main(){

    int runs = 10;
    size_t size = 1000000;
    for(int i = 0; i < runs; i++){
        runtest(size, i);
    }
    return 0;
}

void runtest(size_t size, int countRuns){

    // Create an object of the BFF class
    BFFTEST<uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift> myFilter(size);

    // Create an object of the BFFwrapping class
    BFFwrapping<uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift> myFilter_wrapping(size);

    // Create dataset
    vector<uint64_t> data(size);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }

    // Regular BFF
    // Call the populate function
    bool success = myFilter.populate(data, data.size(), countRuns);

    // Wrapping BFF
    // Call the populate function
    success = myFilter_wrapping.populate(data, data.size(), countRuns);
}
