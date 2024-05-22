#include<stdio.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
int get_matrix_triples(int n){
    vector<int> a;
    vector<vector<int>> sum={{0,0,0}};
    vector<vector<int>> sum2={{0,0,0}};
    for (int i=1;i<=n;i++)
    {
        a.push_back((i*i-i+1)%3);
        sum.push_back(sum[sum.size()-1]);
        sum[i][a[i-1]]+=1;
    }
    for (int times=1;times<3;times++)
    {
    for (int i=1;i<=n;i++)
    {
        sum2.push_back(sum2[sum2.size()-1]);
        if (i>=1)
        for (int j=0;j<=2;j++)
            sum2[i][(a[i-1]+j)%3]+=sum[i-1][j];
    }
    sum=sum2;
    sum2={{0,0,0}};
    }

    return sum[n][0];
}
