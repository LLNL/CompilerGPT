#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<float> rescale_to_unit(vector<float> numbers){ 
    float min=100000,max=-100000;
    for (int i=0;i<numbers.size();i++)
        {
            if (numbers[i]<min) min=numbers[i];
            if (numbers[i]>max) max=numbers[i];
        }
    for (int i=0;i<numbers.size();i++)
        numbers[i]=(numbers[i]-min)/(max-min);
    return numbers;
}
