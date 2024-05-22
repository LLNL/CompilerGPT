#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool iscuber(int a){
    for (int i=0;i*i*i<=abs(a);i++)
        if (i*i*i==abs(a)) return true;
    return false;
}
