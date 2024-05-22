#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
bool same_chars(string s0,string s1){
    for (int i=0;i<s0.length();i++)
    if (find(s1.begin(),s1.end(),s0[i])==s1.end())
        return false;
    for (int i=0;i<s1.length();i++)
    if (find(s0.begin(),s0.end(),s1[i])==s0.end())
        return false;
    return true;   
}
