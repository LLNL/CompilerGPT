#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> pluck(vector<int> arr){
    vector<int> out={};
    for (int i=0;i<arr.size();i++)
    if (arr[i]%2==0 and (out.size()==0 or arr[i]<out[0]))
        out={arr[i],i};
    return out;
}
