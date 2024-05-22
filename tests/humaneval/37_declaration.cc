#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<float> sort_even(vector<float> l){
    vector<float> out={};
    vector<float> even={};
    for (int i=0;i*2<l.size();i++)
        even.push_back(l[i*2]);
    sort(even.begin(),even.end());
    for (int i=0;i<l.size();i++)
    {
        if (i%2==0) out.push_back(even[i/2]);
        if (i%2==1) out.push_back(l[i]);
    }
    return out;
}
