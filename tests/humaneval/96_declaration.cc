#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> count_up_to(int n){
    vector<int> out={};
    int i,j;
    for (i=2;i<n;i++)
        if (out.size()==0) {out.push_back(i);}
        else
        {
            bool isp=true;
            for (j=0;out[j]*out[j]<=i;j++)
                if (i%out[j]==0) isp=false;
            if (isp) out.push_back(i);
        }
    return out;
}
