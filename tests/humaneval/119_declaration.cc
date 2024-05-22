#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string match_parens(vector<string> lst){
    string l1=lst[0]+lst[1];
    int i,count=0;
    bool can=true;
    for (i=0;i<l1.length();i++)
        {
            if (l1[i]=='(') count+=1;
            if (l1[i]==')') count-=1;
            if (count<0) can=false;
        }
    if (count!=0) return "No";
    if (can==true) return "Yes";
    l1=lst[1]+lst[0];
    can=true;
    for (i=0;i<l1.length();i++)
        {
            if (l1[i]=='(') count+=1;
            if (l1[i]==')') count-=1;
            if (count<0) can=false;
        }
    if (can==true) return "Yes";
    return "No";
}
