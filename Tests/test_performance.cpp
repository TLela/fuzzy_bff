#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include "../fuzzy_bff.h" // Include the header file
#include "../lsh_bitsample.h"
#include "../timer.h"
#include "../bff.h"

using namespace std;

void fuzzyBFFTest(size_t size, size_t testsize, vector<size_t>& filtersize, vector<double>& constructiontime, vector<double>& querytime_fp, vector<double>& querytime_fn, vector<uint64_t>& data, vector<uint64_t>& closedata, vector<uint64_t>& fardata, vector<int>& fpcount, vector<int>& fncount, vector<double>& expected_fp, vector<double>& expected_fn);
void DataGeneration(size_t maxsize, size_t maxtestsize, int r_1, int r_2, vector<uint64_t>& data, vector<uint64_t>& closedata, vector<uint64_t>& fardata);
void BFFTest(size_t size, size_t testsize, vector<size_t>& filtersize, vector<double>& constructiontime, vector<double>& querytime_fp, vector<double>& querytime_fn, vector<uint64_t>& data, vector<uint64_t>& fardata, vector<int>& fpcount, vector<int>& fncount);

int main(){

    // Define array to hold data
    vector<size_t> filtersize;
    vector<double> constructiontime;
    vector<double> querytime_fp;
    vector<double> querytime_fn;
    vector<int> fpcount;
    vector<int> fncount;
    vector<double> expected_fp;
    vector<double> expected_fn;

    // Define the size of the filter and the size of the test set
    vector<size_t> size = {10000};//, 20000, 40000, 80000, 160000, 320000, 640000};
    vector<size_t> testsize = {10000};//, 10000, 10000, 10000, 10000, 10000, 10000};

    // Generate data
    vector<uint64_t> data(size.back());
    vector<uint64_t> closedata(testsize.back());
    vector<uint64_t> fardata(testsize.back());
    int r_1 = 3;
    int r_2 = 15;
    DataGeneration(size.back(),testsize.back(), r_1, r_2, data, closedata, fardata);

    for(int i = 0; i < size.size(); i++){
        fuzzyBFFTest(size[i], testsize[i], filtersize, constructiontime, querytime_fp, querytime_fn, data, closedata, fardata, fpcount, fncount, expected_fp, expected_fn);
        fuzzyBFFTest(size[i], testsize[i], filtersize, constructiontime, querytime_fp, querytime_fn, data, closedata, fardata, fpcount, fncount, expected_fp, expected_fn);
        fuzzyBFFTest(size[i], testsize[i], filtersize, constructiontime, querytime_fp, querytime_fn, data, closedata, fardata, fpcount, fncount, expected_fp, expected_fn);
    }
    for(int i = 0; i < size.size(); i++){
        BFFTest(size[i], testsize[i], filtersize, constructiontime, querytime_fp, querytime_fn, data, fardata, fpcount, fncount);
        BFFTest(size[i], testsize[i], filtersize, constructiontime, querytime_fp, querytime_fn, data, fardata, fpcount, fncount);
        BFFTest(size[i], testsize[i], filtersize, constructiontime, querytime_fp, querytime_fn, data, fardata, fpcount, fncount);
    }

    // duplicat size and testsize for easier printing
    size.insert(size.end(), size.begin(), size.end());
    testsize.insert(testsize.end(), testsize.begin(), testsize.end());
    // write results to file
    ofstream myfile("Results/compare_BFF_fBFF.txt");
    myfile << "Size,Testsize,Construction Time,Query Time FP,Query Time FN\n";
    for(int i = 0; i < size.size()/2; i++){
        myfile << size[i] << "," << testsize[i] << ","  << filtersize[3*i] << "," << constructiontime[3*i] << "," << querytime_fp[3*i] << "," << querytime_fn[3*i] << ","  << fpcount[3*i] << ","  << fncount[3*i] << "," << expected_fp[3*i] << "," << expected_fn[3*i] << endl;
        myfile << size[i] << "," << testsize[i] << ","  << filtersize[3*i+1] << "," << constructiontime[3*i+1] << "," << querytime_fp[3*i+1] << "," << querytime_fn[3*i+1] << ","  << fpcount[3*i+1] << ","  << fncount[3*i+1] << "," << expected_fp[3*i+2] << "," << expected_fn[3*i+2] << endl;
        myfile << size[i] << "," << testsize[i] << ","  << filtersize[3*i+2] << "," << constructiontime[3*i+2] << "," << querytime_fp[3*i+2] << "," << querytime_fn[3*i+2] << ","  << fpcount[3*i+2] << ","  << fncount[3*i+2] << "," << expected_fp[3*i+2] << "," << expected_fn[3*i+2] << endl;
    }
    for(int i = size.size()/2; i < size.size(); i++){
        myfile << size[i] << "," << testsize[i] << ","  << filtersize[3*i] << "," << constructiontime[3*i] << "," << querytime_fp[3*i] << "," << querytime_fn[3*i] << ","  << fpcount[3*i] << ","  << fncount[3*i] << endl;
        myfile << size[i] << "," << testsize[i] << ","  << filtersize[3*i+1] << "," << constructiontime[3*i+1] << "," << querytime_fp[3*i+1] << "," << querytime_fn[3*i+1] << ","  << fpcount[3*i+1] << ","  << fncount[3*i+1] << endl;
        myfile << size[i] << "," << testsize[i] << ","  << filtersize[3*i+2] << "," << constructiontime[3*i+2] << "," << querytime_fp[3*i+2] << "," << querytime_fn[3*i+2] << ","  << fpcount[3*i+2] << ","  << fncount[3*i+2] << endl;

    }

    return 0;
}

void DataGeneration(size_t maxsize, size_t maxtestsize, int r_1, int r_2, vector<uint64_t>& data, vector<uint64_t>& closedata, vector<uint64_t>& fardata){

    // Generate random keys to populate the filter
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    // Prepare data to populate
    std::unordered_set<uint64_t> keys;
    while(keys.size() != maxsize){
        keys.insert(dis(gen));
    }
    data = vector<uint64_t>(keys.begin(), keys.end());
    
    // Generate keys close to the above set to check false negatives
    // flip r_1 fraction of bits
    std::uniform_int_distribution<int> close(0, 63);
    std::unordered_set<int> flipped_bits; 
    for(int i = 0; i < maxtestsize; i++){
        while(flipped_bits.size() < r_1){ 
            flipped_bits.insert(close(gen));
        }
        closedata[i] = data[i];
        for (int flipped_bit : flipped_bits) {
            closedata[i] ^= (1ULL << flipped_bit);
        }  
        flipped_bits.clear();   
    }

    // Generate random keys and check that they differ at least in r_2+1 bits for all data points
    std::unordered_set<uint64_t> unorderedfar_keys;
    uint64_t candidate = 0;
    while(unorderedfar_keys.size() != maxtestsize){
        candidate = dis(gen);
        bool found = false;
        while(found!=true){
            int count = 0;
            for(int j = 0; j < maxsize; j++){
                int diff = __builtin_popcountll(candidate ^ data[j]);
                if(diff < r_2 + 1){
                    candidate = dis(gen);
                    //cout << "Count: " << count << endl;
                    break;
                }
                count++;
            }
            if(count == maxsize){
                found = true;
                unorderedfar_keys.insert(candidate);
                //cout << "Found far data point: " << i << endl;
            }
        }
    }
    cout << "Data generation completed." << endl;

}

void fuzzyBFFTest(size_t size, size_t testsize, vector<size_t>& filtersize, vector<double>& constructiontime, vector<double>& querytime_fp, vector<double>& querytime_fn, vector<uint64_t>& data, vector<uint64_t>& closedata, vector<uint64_t>& fardata, vector<int>& fpcount, vector<int>& fncount, vector<double>& expected_fp, vector<double>& expected_fn){ 
    
    // Construct and populate the filter
    Timer construction;
    fuzzyBFF<uint64_t, uint64_t, uint32_t, hashing::SimpleMixSplit, BitSampleLSH> myFilter;
    bool success = myFilter.populate(data, size);
    double time = construction.Stop();
    constructiontime.push_back(time);
    filtersize.push_back(myFilter.arrayLength);


    // Get expected false positive and false negative rates
    // print expected total fp/fn 
    BitSampleLSH lsh;
    expected_fp.push_back(testsize * (size * lsh.p_2 + pow(2,-64)));
    expected_fn.push_back(testsize * (1-(lsh.p_1 + pow(2,-64))));

    int fncount_tmp = 0;
    int fpcount_tmp = 0;
    double membership = 0;

    // Check for false negatives
    double time_fn = 0;
    for (int i = 0; i < testsize; i++) {
        Timer query_fn;
        membership = myFilter.membership(closedata[i]);
        time_fn += query_fn.Stop();
        if (membership == false) {
            fncount_tmp++;            
        }
    }
    querytime_fn.push_back(time_fn/testsize);
    fncount.push_back(fncount_tmp);

    // Check for false positives
    double time_fp = 0;
    for (int i = 0; i < testsize; i++) {
        Timer query_fp;
        membership = myFilter.membership(fardata[i]);
        time_fp += query_fp.Stop();
        if (membership == true) {
            fpcount_tmp++;
        }
    }
    querytime_fp.push_back(time_fp/testsize);
    fpcount.push_back(fpcount_tmp);
}

void BFFTest(size_t size, size_t testsize, vector<size_t>& filtersize, vector<double>& constructiontime, vector<double>& querytime_fp, vector<double>& querytime_fn, vector<uint64_t>& data, vector<uint64_t>& fardata, vector<int>& fpcount, vector<int>& fncount){ 
     
     // Create and populate the filter
     Timer popTime;
     BFF<uint64_t, uint32_t, hashing::TwoIndependentMultiplyShift> myFilter(size);
     bool success = myFilter.populate(data, size);
     double time = popTime.Stop();
     constructiontime.push_back(time);
     filtersize.push_back(myFilter.arrayLength);
 
     int fncount_tmp = 0;
     int fpcount_tmp = 0;
     double membership = 0;

    // Check for false negatives
    double time_fn = 0;
    for (int i = 0; i < testsize; i++) {
        Timer query_fn;
        membership = myFilter.membership(data[i]);
        time_fn += query_fn.Stop();
        if (membership == false) {
            fncount_tmp++;            
        }
    }
    querytime_fn.push_back(time_fn/testsize);
    fncount.push_back(fncount_tmp);

    // Check for false positives
    double time_fp = 0;
    for (int i = 0; i < testsize; i++) {
        Timer query_fp;
        membership = myFilter.membership(fardata[i]);
        time_fp += query_fp.Stop();
        if (membership == true) {
            fpcount_tmp++;
        }
    }
    querytime_fp.push_back(time_fp/testsize);
    fpcount.push_back(fpcount_tmp);
}