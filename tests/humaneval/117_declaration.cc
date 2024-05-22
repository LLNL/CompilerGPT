#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<string> select_words(string s,int n){
    string vowels="aeiouAEIOU";
    string current="";
    vector<string> out={};
    int numc=0;
    s=s+' ';
    for (int i=0;i<s.length();i++)
        if (s[i]==' ')
        {
            if (numc==n) out.push_back(current);
            current="";
            numc=0;
        }
        else
        {
            current=current+s[i];
            if ((s[i]>=65 and s[i]<=90) or (s[i]>=97 and s[i]<=122))
            if (find(vowels.begin(),vowels.end(),s[i])==vowels.end())
                numc+=1;
        }
    return out;
}
