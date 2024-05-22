#include<stdio.h>
#include<string>
#include<algorithm>
using namespace std;
#include<math.h>
#include<stdlib.h>
bool simplify(string x,string n){
    int a,b,c,d,i;
    for (i=0;i<x.size();i++)
        if (x[i]=='/') 
        {
            a=atoi(x.substr(0,i).c_str());
            b=atoi(x.substr(i+1).c_str());
        }
    for (i=0;i<n.size();i++)
        if (n[i]=='/') 
        {
            c=atoi(n.substr(0,i).c_str());
            d=atoi(n.substr(i+1).c_str());
        }
    if ((a*c)%(b*d)==0) return true;
    return false;
}
