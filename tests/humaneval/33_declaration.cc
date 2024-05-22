#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> sort_third(vector<int> l){
    vector<int> third={};
    int i;
    for (i=0;i*3<l.size();i++)
        third.push_back(l[i*3]);
    
    sort(third.begin(),third.end());

    vector<int> out={};
    for (i=0;i<l.size();i++)
    {
        if (i%3==0) {out.push_back(third[i/3]);}
        else out.push_back(l[i]);
    }
    return out;

}
