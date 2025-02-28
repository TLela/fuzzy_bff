#include <iostream>
#include <cassert>

// Test to compare the parameters of the original implementation and our own implementation

// Function prototypes
void testFunction1();
void testFunction2();

int main() {
    std::cout << "Running tests..." << std::endl;
    for(int size = 0; size < 200; size++){
        
        size_t segmentLength1 = 1L << (int)floor(log(size) / log(3.33) + 2.25);

        // TODO: check why/if needed
        if (segmentLength1 > (1 << 18)) {
            segmentLength1 = (1 << 18);
        }
        // Filter needs to be bigger than size of input set
        double factor1 = fmax(1.125, 0.875 -  + 0.25 * log(1000000) / log(size));
        size_t arrayLength1 = factor1 * size;

        // We need to fit an integer number of segments in the filter
        size_t segmentCount1 = ((arrayLength1 + segmentLength1 - 1) / segmentLength1);

        // For very small set sizes
        if(segmentCount1 < 3){
            segmentCount1 = 3;
        }
        
        // Size of the logical filter array
        arrayLength1 = segmentCount1 * segmentLength1;
        
        // Parameters used for getHashFromHash function
        size_t segmentLengthMask1 = segmentLength1 - 1;
        size_t segmentCountLength1 = (segmentCount1 - 2) * segmentLength1;



        int arity = 3;
        size_t segmentLength2 = 1L << (int)floor(log(size) / log(3.33) + 2.25);
        // the current implementation hardcodes a 18-bit limit to
        // to the segment length.
        if (segmentLength2 > (1 << 18)) {
            segmentLength2 = (1 << 18);
        }
        double sizeFactor2 = fmax(1.125, 0.875 + 0.25 * log(1000000) / log(size));
        size_t capacity2 = size * sizeFactor2;
        size_t segmentCount2 =
            (capacity2 + segmentLength2 - 1) / segmentLength2 - (arity - 1);
        size_t arrayLength2 = (segmentCount2 + arity - 1) * segmentLength2;
        size_t segmentLengthMask2 = segmentLength2 - 1;
        segmentCount2 =
            (arrayLength2 + segmentLength2 - 1) / segmentLength2;
        segmentCount2 =
            segmentCount2 <= arity - 1 ? 1 : segmentCount2 - (arity - 1);
        arrayLength2 = (segmentCount2 + arity - 1) * segmentLength2;
        size_t segmentCountLength2 = segmentCount2 * segmentLength2;


        // Compare parameters:

        if(segmentLength1 != segmentLength2 || arrayLength1 != arrayLength2 || segmentCount1-2 != segmentCount2 || segmentCountLength1 != segmentCountLength2) {
            std::cout << "size: " << size << std::endl;
            std::cout << "segmentLength: " << segmentLength1 << " vs " << segmentLength2 << std::endl;
            std::cout << "arrayLength: " << arrayLength1 << " vs " << arrayLength2 << std::endl;
            std::cout << "segmentCount: " << segmentCount1-2 << " vs " << segmentCount2 << std::endl;
            std::cout << "segmentCountLength: " << segmentCountLength1 << " vs " << segmentCountLength2 << std::endl;
        }
    }
    
    return 0;
}

