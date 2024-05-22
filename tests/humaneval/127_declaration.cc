#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
string intersection( vector<int> interval1,vector<int> interval2){
    int inter1,inter2,l,i;
    inter1=max(interval1[0],interval2[0]);
    inter2=min(interval1[1],interval2[1]);
    l=inter2-inter1;
    if (l<2) return "NO";
    for (i=2;i*i<=l;i++)
        if (l%i==0) return "NO";
    return "YES";
}
