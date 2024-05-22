#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int hex_key(string num){
    string key="2357BD";
    int out=0;
    for (int i=0;i<num.length();i++)
    if (find(key.begin(),key.end(),num[i])!=key.end()) out+=1;
    return out;
}
