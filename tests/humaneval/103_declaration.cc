#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string rounded_avg(int n,int m){
    if (n>m) return "-1";
    int num=(m+n)/2;
    string out="";
    while (num>0)
    {
        out=to_string(num%2)+out;
        num=num/2;
    }
    return out;
}
