#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int modp(int n,int p){
    int out=1;
    for (int i=0;i<n;i++)
        out=(out*2)%p;
    return out;
}
