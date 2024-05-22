#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> strange_sort_list(vector<int> lst){
    vector<int> out={};
    sort(lst.begin(),lst.end());
    int l=0,r=lst.size()-1;
    while (l<r)
    {
        out.push_back(lst[l]);
        l+=1;
        out.push_back(lst[r]);
        r-=1;
    }
    if (l==r) out.push_back(lst[l]);
    return out;

}
