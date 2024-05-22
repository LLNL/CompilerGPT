#include<stdio.h>
#include<math.h>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int x_or_y(int n,int x,int y){
    bool isp=true;
    if (n<2) isp=false;
    for (int i=2;i*i<=n;i++)
    if (n%i==0) isp=false;
    if (isp) return x;
    return y;
}
