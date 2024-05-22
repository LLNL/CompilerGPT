#include<stdio.h>
#include<math.h>
#include<string>
#include<map>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool check_dict_case(map<string,string> dict){
    map<string,string>::iterator it;
    int islower=0,isupper=0;
    if (dict.size()==0) return false;
    for (it=dict.begin();it!=dict.end();it++)
    {
        string key=it->first;
    
        for (int i=0;i<key.length();i++)
        {
            if (key[i]<65 or (key[i]>90 and key[i]<97) or key[i]>122) return false;
            if (key[i]>=65 and key[i]<=90) isupper=1;
            if (key[i]>=97 and key[i]<=122) islower=1;
            if (isupper+islower==2) return false;
        }

    }
    return true;
}
