#include<stdio.h>
#include<string>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
bool cycpattern_check(string a,string b){
    for (int i=0;i<b.size();i++)
    {
        string rotate=b.substr(i)+b.substr(0,i);
        if (a.find(rotate)!=string::npos) return true;
    }
    return false;

}
