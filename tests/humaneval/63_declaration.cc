#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int fibfib(int n){
    int ff[100];
    ff[0]=0;
    ff[1]=0;
    ff[2]=1;
    for (int i=3;i<=n;i++)
        ff[i]=ff[i-1]+ff[i-2]+ff[i-3];
    return ff[n];

}
