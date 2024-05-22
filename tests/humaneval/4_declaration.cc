#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
float mean_absolute_deviation(vector<float> numbers){
    float sum=0;
    float avg,msum,mavg;
    int i=0;
    for (i=0;i<numbers.size();i++)
        sum+=numbers[i];
    avg=sum/numbers.size();
    msum=0;
    for (i=0;i<numbers.size();i++)
        msum+=abs(numbers[i]-avg);
    return msum/numbers.size();
}
