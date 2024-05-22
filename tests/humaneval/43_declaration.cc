#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool pairs_sum_to_zero(vector<int> l){
    for (int i=0;i<l.size();i++)
    for (int j=i+1;j<l.size();j++)
    if (l[i]+l[j]==0) return true;
    return false;
}
