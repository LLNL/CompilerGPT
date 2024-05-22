#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool is_simple_power(int x,int n){
    int p=1,count=0;
    while (p<=x and count<100)
    {
        if (p==x) return true;
        p=p*n;count+=1;
    }
    return false;
}
