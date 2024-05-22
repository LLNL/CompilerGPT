#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int fruit_distribution(string s,int n){
    string num1="",num2="";
    int is12;
    is12=0;
    for (int i=0;i<s.size();i++)
        
        if (s[i]>=48 and s[i]<=57)
        {
            if (is12==0) num1=num1+s[i];
            if (is12==1) num2=num2+s[i];
        }
        else
          if (is12==0 and num1.length()>0) is12=1;
    return n-atoi(num1.c_str())-atoi(num2.c_str());

}
