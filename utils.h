#include <assert.h>

namespace cs457
{

    //https://stackoverflow.com/questions/3407012/c-rounding-up-to-the-nearest-multiple-of-a-number
    int roundUpMultipleOfTwo(int numToRound, int multiple) 
    {
        //positive only 
          assert(multiple && ((multiple & (multiple - 1)) == 0));
          return (numToRound + multiple - 1) & -multiple;
    }
}