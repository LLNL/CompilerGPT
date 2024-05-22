#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int count_distinct_characters(string str){ 
    vector<char> distinct={};
    transform(str.begin(),str.end(),str.begin(),::tolower);
    for (int i=0;i<str.size();i++)
    {
        bool isin=false;
        for (int j=0;j<distinct.size();j++)
            if (distinct[j]==str[i])
                isin=true;
        if (isin==false) distinct.push_back(str[i]);

    }
    return distinct.size();
}
