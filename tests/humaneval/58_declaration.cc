#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> common(vector<int> l1,vector<int> l2){
    vector<int> out={};
    for (int i=0;i<l1.size();i++)
        if (find(out.begin(),out.end(),l1[i])==out.end())
            if (find(l2.begin(),l2.end(),l1[i])!=l2.end())
                out.push_back(l1[i]);
    sort(out.begin(),out.end());
    return out;
}
