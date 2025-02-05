# fuzzy_bff
Semester project

Bug in populate function - somewhere before line 225 (bff.h)
construction fails more often than expected

## How to use BFF

The populate function of the filter has type: 
```c 
bool BFF<ItemType, FingerprintType, HashFamily>::populate(const ItemType* data, size_t length)
```

How to use BFF

```cpp
  // Define the size of the filter
  size_t size = 100000;

  // Create an object of the fuzzyBFF class
  BFF<uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift> myFilter(size);

  // Print some of the initialized values (optional)
  cout << "Filter details:" << endl;
  cout << "Size: " << myFilter.size << endl;
  cout << "Segment Length: " << myFilter.segmentLength << endl;
  cout << "Segment Count: " << myFilter.segmentCount << endl;
  cout << "Array Length: " << myFilter.arrayLength << endl;
  cout << "Filter Length: " << myFilter.filterLength << endl;


  // Populate the function with keys in data
  vector<uint64_t> data(size);
  bool success = myFilter.populate(data, data.size());

  // Get the pointer to the filter
  uint32_t* filter = myFilter.getFilter();

  // Check membership of a key
  uint64_t key;
  bool isMember = myFilter.membership(key);
      
```
## How to use fuzzyBFF
Note that here we don't need to specify the set size in advance since the size of the final set depends on the LSH we use.

For debugging see `debug.cpp`. 
Compile this file and execute it with `./debug <filtersize>`.

The populate function of the filter has type: 
```c 
bool fuzzyBFF<InputType, ItemType, FingerprintType, HashFamily, LSHType>::populate(const InputType* data, size_t length)
```

How to use fuzzyBFF:

```cpp
  // Define the size of the filter
  size_t size = 100000;

  // Create an object of the fuzzyBFF class
  fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift, ExampleLSH> myFilter;

  // Populate the Filter with keys in data
  vector<uint64_t> data(size);
  bool success = myFilter.populate(data, data.size());

  // Print some of the initialized values (optional)
  cout << "Filter details:" << endl;
  cout << "Size: " << myFilter.size << endl;
  cout << "Segment Length: " << myFilter.segmentLength << endl;
  cout << "Segment Count: " << myFilter.segmentCount << endl;
  cout << "Array Length: " << myFilter.arrayLength << endl;
  cout << "Filter Length: " << myFilter.filterLength << endl;

  // Get the pointer to the filter
  uint32_t* filter = myFilter.getFilter();

  // Check membership of a key
  uint64_t newkey;
  bool isMember = myFilter.membership(newkey);
```



## Worklog
- implemented populate function and membership function
- setup file for fuzzy bff - used std::unordered_set to remove duplicates - seems to work
- why do we have false negatives? -> mistake in code of populate function -> solved
- revisit fuzzy bff and check if it works -> changed initialization, now after projecting to new set using lsh
- create lsh classes
- fixed mistake in memb function of fuzzy bff -> now checks memb of lsh(item) instead of item

Next:
- readme and comment on bug
- debug
- decide on example lsh and implement 2-3 (mo)
  - Hemming/Euclidean
  - read up on them 
  - write up lsh
- test it (mo/thu)
- set up testing environment (thu/fr)
  - for time and space (thu/fr)
- decide on what to compare it to (fr)
- implement comparison(fr/tue)
- clean up (tue/wed)
- back to theory...
- ...