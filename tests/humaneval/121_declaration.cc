#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int solutions(vector<int> lst){
    int sum=0;
    for (int i=0;i*2<lst.size();i++)
        if (lst[i*2]%2==1) sum+=lst[i*2];
    return sum;
}
