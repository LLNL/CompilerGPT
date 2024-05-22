#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
string remove_vowels(string text){
    string out="";
    string vowels="AEIOUaeiou";
    for (int i=0;i<text.length();i++)
        if (find(vowels.begin(),vowels.end(),text[i])==vowels.end())
            out=out+text[i];
    return out;

}
