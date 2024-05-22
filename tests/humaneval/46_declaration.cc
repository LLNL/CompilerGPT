#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int fib4(int n){
    int f[100];
    f[0]=0;
    f[1]=0;
    f[2]=2;
    f[3]=0;
    for (int i=4;i<=n;i++)
    {
        f[i]=f[i-1]+f[i-2]+f[i-3]+f[i-4];
    }
    return f[n];
}
