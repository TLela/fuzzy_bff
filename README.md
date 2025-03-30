# Fuzzy BFFs: Distance-Sensitive Binary Fuse Filters
## Table of Contents
- [Fuzzy BFFs: Distance-Sensitive Binary Fuse Filters](#fuzzy-bffs-distance-sensitive-binary-fuse-filters)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Filter Classes](#filter-classes)
    - [Binary Fuse Filter](#binary-fuse-filter)
    - [Wrapping Binary Fuse Filter](#wrapping-binary-fuse-filter)
    - [Fuzzy Binary Fuse Filter](#fuzzy-binary-fuse-filter)
  - [LSH Classes](#lsh-classes)

## Introduction
This repository is part of the semester project 'Fuzzy BFFs: Distance-Sensitive Binary Fuse Filters', completed at the Institute of Information Security at ETH Zurich. It contains implementations for the Binary fuse filter as introduced by [Graf and Lemire](https://doi.org/10.1145/3510449), a variation of this filter which we call the wrapping Binary fuse filter, and the fuzzy Binary fuse filter which was proposed as part of this project. We provide several tests to measure the performance and reveal the inner workings of these filters.

## Filter Classes
### Binary Fuse Filter
For this class we used and adapted the original implementation by [Graf and Lemire](https://github.com/FastFilter/fastfilter_cpp/blob/master/src/xorfilter/3wise_xor_binary_fuse_filter_naive.h). There are some minor details that we changed, like how some of the involved structures are represented in the code. The one major thing we changed is the calculation of parameters (number of segments, size of segments, size of filter,...). By closely following the description of parameter calculation presented in the original paper, we were able to simplify this part of the code significantly.

Run and test this class by running
```
g++ test_bff.cpp -o test -std=c++17 
./test
```

For plotting the singleton distribution overtime during the filter construction, run
```
cd Tests
g++ test_wrapping.cpp -o test -std=c++17 
./test
cd Plotting
python3 ./singletonplot_overtime_relative.py
python3 ./singletonplot_overtime_total.py
```
The plots will be available in the [Plots](https://github.com/TLela/fuzzy_bff/tree/main/Tests/Plots) folder.

### Wrapping Binary Fuse Filter
This class was created to showcase the importance of the 'fuse' part of the Binary fuse filter. For the regular Binary fuse filter we simply map to three distinct segments such that they always are in the bounds of the array. For the wrapping Binary fuse filter we consider an alternative where we first map to any segment of the array, then add the two consequtive ones. If they are out of bounds, we wrap around to the segments at the beginning of the array. While the construction of the Binary fuse filter succeeds nearly all the time, the construction of the wrapping version fails repeatedly. This is due to the difference in singleton distribution during the construction. Consult the write-up for more details.

Compare the singleton distribution by running
```
cd Tests
g++ test_wrapping.cpp -o test -std=c++17 
./test
cd Plotting
python3 ./singletonplot_comparison.py
```
The plot will be available in the [Plots](https://github.com/TLela/fuzzy_bff/tree/main/Tests/Plots) folder.

### Fuzzy Binary Fuse Filter
This filter was built by combining Locality-senstive hash (LSH) functions and Binary fuse filters. Specify your desired LSH during initialisation of the filter. We implemented a Bit-sampling hash as an example. 

Run the filter using the Bit-sampling hash
```
cd Tests
g++ test_fuzzybff.cpp -o test -std=c++17 
./test
```

Compare the performance of the original filter with the fuzzy version
```
cd Tests
g++ test_performance.cpp -o test -std=c++17 
./test
cd Plotting
python3 ./perfromanceplot.py
```
The plots will be available in the [Plots](https://github.com/TLela/fuzzy_bff/tree/main/Tests/Plots) folder.

## LSH Classes
There is currently only the Bit-sampling hash implemented, but one can easily implement an alternative Locality-sensitve hash (LSH) using the lsh class in [lsh.h](https://github.com/TLela/fuzzy_bff/blob/main/lsh.h).