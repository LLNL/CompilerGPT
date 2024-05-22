#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<float> get_positive(vector<float> l){
    vector<float> out={};
    for (int i=0;i<l.size();i++)
        if (l[i]>0) out.push_back(l[i]);
    return out;
}
