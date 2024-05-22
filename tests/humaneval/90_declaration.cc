#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int next_smallest(vector<int> lst){
    sort(lst.begin(),lst.end());
    for (int i=1;i<lst.size();i++)
        if (lst[i]!=lst[i-1]) return lst[i];
    return -1;
}
