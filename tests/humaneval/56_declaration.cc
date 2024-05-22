#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool correct_bracketing(string brackets){
    int level=0;
    for (int i=0;i<brackets.length();i++)
    {
        if (brackets[i]=='<') level+=1;
        if (brackets[i]=='>') level-=1;
        if (level<0) return false;
    }
    if (level!=0) return false;
    return true;
}
