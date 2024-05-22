#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> f(int n){
    int sum=0,prod=1;
    vector<int> out={};
    for (int i=1;i<=n;i++)
    {
        sum+=i;
        prod*=i;
        if (i%2==0) out.push_back(prod);
        else out.push_back(sum);
    } 
    return out;
}
