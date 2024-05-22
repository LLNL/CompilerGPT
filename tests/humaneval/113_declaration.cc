#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<map>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<string> odd_count(vector<string> lst){
    vector<string> out={};
    for (int i=0;i<lst.size();i++)
        {
            int sum=0;
            for (int j=0;j<lst[i].length();j++)
                if (lst[i][j]>=48 and lst[i][j]<=57 and lst[i][j]%2==1)
                sum+=1;
            string s="the number of odd elements in the string i of the input.";
            string s2="";
            for (int j=0;j<s.length();j++)
                if (s[j]=='i') s2=s2+to_string(sum);
                else s2=s2+s[j];
            out.push_back(s2);
        }
    return out;
}
