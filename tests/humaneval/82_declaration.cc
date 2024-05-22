#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool prime_length(string str){
    int l,i;
    l=str.length();
    if (l<2) return false;
    for (i=2;i*i<=l;i++)
    if (l%i==0) return false;
    return true;
}
