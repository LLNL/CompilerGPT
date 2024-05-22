#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<string> words_string(string s){
    string current="";
    vector<string> out={};
    s=s+' ';
    for (int i=0;i<s.length();i++)
     if (s[i]==' ' or s[i]==',')
     {
        if (current.length()>0)
        {
            out.push_back(current);
            current="";
        }
     }
     else current=current+s[i];
     return out;
}
