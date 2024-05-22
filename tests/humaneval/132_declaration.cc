#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
bool is_nested(string str){
    int count=0,maxcount=0;
    for (int i=0;i<str.length();i++)
    {
        if (str[i]=='[') count+=1;
        if (str[i]==']') count-=1;
        if (count<0) count=0;
        if (count>maxcount) maxcount=count;
        if (count<=maxcount-2) return  true;
    }
    return false;
}
