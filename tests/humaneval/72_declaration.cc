#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool will_it_fly(vector<int> q,int w){
    int sum=0;
    for (int i=0;i<q.size();i++)
    {
        if (q[i]!=q[q.size()-1-i]) return false;
        sum+=q[i];
    }
    if (sum>w) return false;
    return true;
}
