#include<stdio.h>
#include<math.h>
#include<string>
#include<map>
using namespace std;
#include<algorithm>
#include<stdlib.h>
map<char,int> histogram(string test){
    map<char,int> count={},out={};
    map <char,int>::iterator it;
    int max=0;
    for (int i=0;i<test.length();i++)
        if (test[i]!=' ')
        {
            count[test[i]]+=1;
            if (count[test[i]]>max) max=count[test[i]];
        }
    for (it=count.begin();it!=count.end();it++)
    {
        char w1=it->first;
        int w2=it->second;
        if (w2==max) out[w1]=w2;
    }
    return out;
}
