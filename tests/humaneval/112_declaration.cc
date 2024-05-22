#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<string> reverse_delete(string s,string c){
    string n="";
    for (int i=0;i<s.length();i++)
    if (find(c.begin(),c.end(),s[i])==c.end())
        n=n+s[i]; 
    if (n.length()==0) return {n,"True"};
    string w(n.rbegin(),n.rend());
    if (w==n) return {n,"True"};
    return {n,"False"};
}
