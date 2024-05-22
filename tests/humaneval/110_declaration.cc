#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string exchange(vector<int> lst1,vector<int> lst2){
    int num=0;
    for (int i=0;i<lst1.size();i++)
    if (lst1[i]%2==0) num+=1;
    for (int i=0;i<lst2.size();i++)
    if (lst2[i]%2==0) num+=1;
    if (num>=lst1.size()) return "YES";
    return "NO";
}
