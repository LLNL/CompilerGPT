#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int largest_divisor(int n){
    for (int i=2;i*i<=n;i++)
        if (n%i==0) return  n/i;
    return 1;

}
