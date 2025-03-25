#include <iostream>
#include <vector>
#include <cmath>
#include "../bff_wrapping.h" // Include the header file
#include "../timer.h"
#include <random>

// TEST: Compare the BFF and BFFwrapping classes.
// We populate the filters with random data and check membership of the data.
// Due to the wrapping the latter class will fail repeatedly. This phenomenon is described further in the writeup.
// This test will visualize the distribution of mapped values to the filter array for both classes.


using namespace std;
void runtest(size_t size, int countRuns);

int main(){

    int runs = 20;
    size_t size = 100000;
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

    // Print some of the initialized values
    printf("BFF\n");
    printf("----------------\n");
    myFilter.printInfo();
    printf("----------------\n");
    printf("----------------\n");
    printf("BFFwrapping\n");
    printf("----------------\n");
    myFilter_wrapping.printInfo();
    printf("----------------\n");
    printf("----------------\n");

    //Print description of table
    cout << "Segm: Segment number" << endl;
    cout << "Singlt: Number of singletons found" << endl;
    cout << "Total: Total number of keys mapped to this semgent" << endl;
    cout << "Ratio: Singletons found / (keys mapped to this segment - remaining keys)" << endl;
    cout << "Remaining: Remaining keys mapped to this segment but no primary location found. This will be all zero if construction succeeded." << endl;

    
    // Create dataset
    vector<uint64_t> data(size);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }

    // Regular BFF
    printf("Populating BFF\n");
    // Call the populate function
    Timer popTime;
    bool success = myFilter.populate(data, data.size(), countRuns);
    double time = popTime.Stop();
    cout << "Populate time: " << time << " microseconds total" << endl;
    cout << "Populate time:" << time/size << " microseconds per item" << endl;

    // Wrapping BFF
    printf("Populating BFFwrapping\n");
    // Call the populate function
    success = myFilter_wrapping.populate(data, data.size(), countRuns);
    time = popTime.Stop(); // Most of the time we exit here bc the construction fails
    cout << "Populate time: " << time << " microseconds total" << endl;
    cout << "Populate time:" << time/size << " microseconds per item" << endl;

}
