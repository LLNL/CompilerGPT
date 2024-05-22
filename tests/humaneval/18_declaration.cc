#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int how_many_times(string str,string substring){
    int out=0;
    if (str.length()==0) return 0;
    for (int i=0;i<=str.length()-substring.length();i++)
    if (str.substr(i,substring.length())==substring)
        out+=1;
    return out;
}
