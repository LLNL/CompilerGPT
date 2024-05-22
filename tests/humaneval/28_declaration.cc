#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string concatenate(vector<string> strings){
    string out="";
    for (int i=0;i<strings.size();i++)
        out=out+strings[i];
    return out;
}
