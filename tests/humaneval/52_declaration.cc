#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool below_threshold(vector<int>l, int t){
    for (int i=0;i<l.size();i++)
        if (l[i]>=t) return false;
    return true;
}
