#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int prime_fib(int n){
    int f1,f2,m;
    f1=1;f2=2;
    int count=0;
    while (count<n)
    {
        f1=f1+f2;
        m=f1;f1=f2;f2=m;
        bool isprime=true;
        for (int w=2;w*w<=f1;w++)
            if (f1%w==0)
            {
             isprime=false; break;
            }
        if (isprime) count+=1;
        if (count==n) return f1;
    }

}
