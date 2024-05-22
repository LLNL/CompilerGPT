#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> maximum(vector<int> arr,int k){
    sort(arr.begin(),arr.end());
    vector<int> out(arr.end()-k,arr.end());
    return out;
}
