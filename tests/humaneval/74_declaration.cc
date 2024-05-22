#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<string> total_match(vector<string> lst1,vector<string> lst2){
    int num1,num2,i;
    num1=0;num2=0;
    for (i=0;i<lst1.size();i++)
        num1+=lst1[i].length();
    for (i=0;i<lst2.size();i++)
        num2+=lst2[i].length();
    if (num1>num2) return lst2;
    return lst1;
}
