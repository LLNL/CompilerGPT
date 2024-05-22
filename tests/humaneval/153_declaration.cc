#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
string Strongest_Extension(string class_name,vector<string> extensions){
    string strongest="";
    int max=-1000;
    for (int i=0;i<extensions.size();i++)
    {
        int strength=0;
        for (int j=0;j<extensions[i].length();j++)
        {
            char chr=extensions[i][j];
            if (chr>=65 and chr<=90) strength+=1;
            if (chr>=97 and chr<=122) strength-=1;
        }
        if (strength>max) 
        {
            max=strength;
            strongest=extensions[i];
        }
    }
    return class_name+'.'+strongest;
}
