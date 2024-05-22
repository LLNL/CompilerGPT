#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
long long double_the_difference(vector<float> lst){
    long long sum=0;
    for (int i=0;i<lst.size();i++)
    if (lst[i]-round(lst[i])<1e-4)
    if (lst[i]>0 and (int)(round(lst[i]))%2==1) sum+=(int)(round(lst[i]))*(int)(round(lst[i]));
    return sum;
}
