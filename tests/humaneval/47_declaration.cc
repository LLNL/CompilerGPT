#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
float median(vector<float> l){
    sort(l.begin(),l.end());
    if (l.size()%2==1) return l[l.size()/2];
    return 0.5*(l[l.size()/2]+l[l.size()/2-1]);
}
