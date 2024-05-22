#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool is_multiply_prime(int a){
    int num=0;
    for (int i=2;i*i<=a;i++)
    while (a%i==0 and a>i)
    {
        a=a/i;
        num+=1;
    }
    if (num==2) return true;
    return false; 
}
