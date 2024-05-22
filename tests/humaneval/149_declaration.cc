#include<stdio.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<math.h>
#include<stdlib.h>
vector<string> sorted_list_sum(vector<string> lst){
    vector<string> out={};
    for (int i=0;i<lst.size();i++)
    if (lst[i].length()%2==0) out.push_back(lst[i]);
    string mid;
    sort(out.begin(),out.end());
    for (int i=0;i<out.size();i++)
    for (int j=1;j<out.size();j++)
    if (out[j].length()<out[j-1].length())
    {
        mid=out[j];out[j]=out[j-1];out[j-1]=mid;
    }
    return out;
}
