#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int vowels_count(string s){
    string vowels="aeiouAEIOU";
    int count=0;
    for (int i=0;i<s.length();i++)
    if (find(vowels.begin(),vowels.end(),s[i])!=vowels.end())
        count+=1;
    if (s[s.length()-1]=='y' or s[s.length()-1]=='Y') count+=1;
    return count;
}
