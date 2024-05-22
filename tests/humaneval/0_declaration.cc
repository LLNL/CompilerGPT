#include<stdio.h>
#include<vector>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool has_close_elements(vector<float> numbers, float threshold){
    int i,j;
    
    for (i=0;i<numbers.size();i++)
    for (j=i+1;j<numbers.size();j++)
    if (abs(numbers[i]-numbers[j])<threshold)
    return true;

    return false;
}

