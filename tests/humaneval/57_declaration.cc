#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool monotonic(vector<float> l){
    int incr,decr;
    incr=0;decr=0;
    for (int i=1;i<l.size();i++)
    {
        if (l[i]>l[i-1]) incr=1;
        if (l[i]<l[i-1]) decr=1;
    }
    if (incr+decr==2) return false;
    return true;
}
