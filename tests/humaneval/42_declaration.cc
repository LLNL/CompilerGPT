#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> incr_list(vector<int> l){
    for (int i=0;i<l.size();i++)
        l[i]+=1;
    return l;
}
