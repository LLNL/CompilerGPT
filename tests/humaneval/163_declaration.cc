#include<stdio.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
vector<int> generate_integers(int a,int b){
    int m;
    if (b<a)
    {
        m=a;a=b;b=m;
    }

    vector<int> out={};
    for (int i=a;i<=b;i++)
    if (i<10 and i%2==0) out.push_back(i);
    return out;
}
