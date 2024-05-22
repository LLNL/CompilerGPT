#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int count_nums(vector<int> n){
    int num=0;
    for (int i=0;i<n.size();i++)
        if (n[i]>0) num+=1;
        else
        {
            int sum=0;
            int w;
            w=abs(n[i]);
            while (w>=10)
            {
                sum+=w%10;
                w=w/10;
            }
            sum-=w;
            if (sum>0) num+=1;
        }
    return num;
}
