#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<string> filter_by_prefix(vector<string> strings, string prefix){
    vector<string> out={};
    for (int i=0;i<strings.size();i++)
        if (strings[i].substr(0,prefix.length())==prefix) out.push_back(strings[i]);
    return out;
}
