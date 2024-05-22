#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<map>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<string> by_length(vector<int> arr){
    map<int,string> numto={{0,"Zero"},{1,"One"},{2,"Two"},{3,"Three"},{4,"Four"},{5,"Five"},{6,"Six"},{7,"Seven"},{8,"Eight"},{9,"Nine"}};
    sort(arr.begin(),arr.end());
    vector<string> out={};
    for (int i=arr.size()-1;i>=0;i-=1)
        if (arr[i]>=1 and arr[i]<=9)
        out.push_back(numto[arr[i]]);
    return out;
}
