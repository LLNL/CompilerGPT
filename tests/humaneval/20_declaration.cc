#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<float> find_closest_elements(vector<float> numbers){
    vector<float> out={};
    for (int i=0;i<numbers.size();i++)
    for (int j=i+1;j<numbers.size();j++)
        if (out.size()==0 or abs(numbers[i]-numbers[j])<abs(out[0]-out[1]))
            out={numbers[i],numbers[j]};
    if (out[0]>out[1])
        out={out[1],out[0]};
    return out;
}
