#include<stdio.h>
#include<string>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
string solve(string s){
    int nletter=0;
    string out="";
    for (int i=0;i<s.length();i++)
    {
        char w=s[i];
        if (w>=65 and w<=90) w=w+32;
        else if (w>=97 and w<=122) w=w-32;
        else nletter+=1;
        out=out+w;
    }
    if (nletter==s.length())
    {
        string p(s.rbegin(),s.rend());
        return p;
    }
    else return out;
}
