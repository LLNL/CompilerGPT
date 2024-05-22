#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int search(vector<int> lst){
    vector<vector<int>> freq={};
    int max=-1;
    for (int i=0;i<lst.size();i++)
    {
        bool has=false;
        for (int j=0;j<freq.size();j++)
            if (lst[i]==freq[j][0]) 
            {
            freq[j][1]+=1;
            has=true;
            if (freq[j][1]>=freq[j][0] and freq[j][0]>max) max=freq[j][0];
            }
        if (not(has)) 
        {
        freq.push_back({lst[i],1});
        if (max==-1 and lst[i]==1) max=1;
        }
    }
    return max;
}
