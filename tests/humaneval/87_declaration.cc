#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<vector<int>> get_row(vector<vector<int>> lst, int x){
    vector<vector<int>> out={};
    for (int i=0;i<lst.size();i++)
    for (int j=lst[i].size()-1;j>=0;j-=1)
    if (lst[i][j]==x) out.push_back({i,j});
    return out;
}
