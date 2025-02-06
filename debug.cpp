#include <iostream>
#include "fuzzy_bff.h"


int main(int argc, char** argv) {

  // Read the size of the filter
  if (argc != 2) {
    std::cout << "Usage: ./debug <size>" << std::endl;
    return 1;
  }
  size_t size = std::stoi(argv[1]);

  // Create an object of the fuzzyBFF class
  fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift, ExampleLSH> myFilter;

  // Populate the Filter with keys in data
  vector<uint64_t> data(size);
  for (size_t i = 0; i < size; i++) {
    data[i] = i;
  }


  bool success = myFilter.populate(data, data.size());

  myFilter.printInfo();

  // Get the pointer to the filter
  uint32_t* filter = myFilter.getFilter();

  // Check membership of a key
  uint64_t newkey;
  bool isMember = myFilter.membership(newkey);



}