#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool any_int(float a,float b,float c){
    if (round(a)!=a) return false;
    if (round(b)!=b) return false;
    if (round(c)!=c) return false;
    if (a+b==c or a+c==b or b+c==a) return true;
    return false;
}
