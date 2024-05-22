#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> unique_digits(vector<int> x){
    vector<int> out={};
    for (int i=0;i<x.size();i++)
        {
            int num=x[i];
            bool u=true;
            if (num==0) u=false;
            while (num>0 and u)
            {
                if (num%2==0) u=false;
                num=num/10;
            }
            if (u) out.push_back(x[i]);
        }
    sort(out.begin(),out.end());
    return out;
}
