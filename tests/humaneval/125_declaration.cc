#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<string> split_words(string txt){
    int i;
    string current="";
    vector<string> out={};
    if (find(txt.begin(),txt.end(),' ')!=txt.end())
    {
        txt=txt+' ';
        for (i=0;i<txt.length();i++)
            if (txt[i]==' ') 
            {
                if (current.length()>0)out.push_back(current); 
                current="";
            }
            else current=current+txt[i];
        return out;
    }
    if (find(txt.begin(),txt.end(),',')!=txt.end())
    {
        txt=txt+',';
        for (i=0;i<txt.length();i++)
            if (txt[i]==',') 
            {
                if (current.length()>0)out.push_back(current); 
                current="";
            }
            else current=current+txt[i];
        return out;
    }
    int num=0;
    for (i=0;i<txt.length();i++)
        if (txt[i]>=97 and txt[i]<=122 and txt[i]%2==0)
            num+=1;
    return {to_string(num)};
}
