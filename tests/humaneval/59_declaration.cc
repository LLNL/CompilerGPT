#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int largest_prime_factor(int n){
    for (int i=2;i*i<=n;i++)
    while (n%i==0 and n>i)  n=n/i;
    return n;
}
