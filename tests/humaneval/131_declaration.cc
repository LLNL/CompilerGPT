#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int digits(int n){
    int prod=1,has=0;
    string s=to_string(n);
    for (int i=0;i<s.length();i++)
        if (s[i]%2==1) 
        {
            has=1;
            prod=prod*(s[i]-48);
        }
    if (has==0) return 0;
    return prod;
}
