#include<stdio.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
vector<int> rolling_max(vector<int> numbers){
    vector<int> out;
    int max=0;
    for (int i=0;i<numbers.size();i++)
    {
        if (numbers[i]>max) max=numbers[i];
        out.push_back(max);
    }
    return out;
}
