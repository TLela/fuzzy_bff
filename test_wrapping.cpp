#include <iostream>
#include <vector>
#include <cmath>
#include "bff_wrapping.h"
#include "timer.h"
#include <random>

// TEST: Compare the BFF and BFFwrapping classes.
// We populate the filters with random data and check membership of the data.
// Due to the wrapping the latter class will fail repeatedly. This phenomenon is described further in the writeup.
// This test will visualize the distribution of mapped values to the filter array for both classes.

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
    size_t size = 10000000;

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

    // Prepare data to populate
    vector<uint64_t> data = GenerateRandom64Fast(size, rand()) ;
    
    // Create set of random keys not in the set
    int notinsetsize = size;
    vector<uint64_t> notinset(notinsetsize);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(size, std::numeric_limits<uint64_t>::max());

    for (int i = 0; i < notinsetsize; ++i) {
        notinset[i] = dis(gen);
    }

    printf("Populating BFF\n");
    // Regular BFF
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
            //cout << "False negative: " << data[i] << endl;
            fncount++;
  
        }
    }

    // Check membership of keys not in set
    for (uint64_t i = 0; i < notinsetsize; ++i) {
        bool isMember = myFilter.membership(notinset[i]);
        // False positive
        if(isMember){
            cout << "False positive: " << notinset[i] << endl;
            fpcount++;
        }
    }

    // fp and fn fraction
    cout << "False negative fraction: " << fncount << "/" << size << endl;
    cout << "False positive fraction: " << fpcount << "/" << notinsetsize << endl;

    // Wrapping BFF
    printf("Populating BFFwrapping\n");
    // Call the populate function
    success = myFilter_wrapping.populate(data, data.size());
    time = popTime.Stop(); // Most of the time we exit here bc the construction fails
    cout << "Populate time: " << time << " microseconds total" << endl;
    cout << "Populate time:" << time/size << " microseconds per item" << endl;

    // Get the pointer to the filter
    filter = myFilter_wrapping.getFilter();

    fncount = 0;
    fpcount = 0;
    
    // Check membership of all the keys
    for (size_t i = 0; i < size; ++i) {
        bool isMember = myFilter_wrapping.membership(data[i]);
        // False negative
        if(!isMember){
            //cout << "False negative: " << data[i] << endl;
            fncount++;
  
        }
    }

    // Check membership of keys not in set
    for (uint64_t i = 0; i < notinsetsize; ++i) {
        bool isMember = myFilter_wrapping.membership(notinset[i]);
        // False positive
        if(isMember){
            cout << "False positive: " << notinset[i] << endl;
            fpcount++;
        }
    }
    
    // fp and fn fraction
    cout << "False negative fraction: " << fncount << "/" << size << endl;
    cout << "False positive fraction: " << fpcount << "/" << notinsetsize << endl;

    return 0;
}
