#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool is_happy(string s){
    if (s.length()<3) return false;
    for (int i=2;i<s.length();i++)
    if (s[i]==s[i-1] or s[i]==s[i-2]) return false;
    return true;
}
