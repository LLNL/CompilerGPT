#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int choose_num(int x,int y){
    if (y<x) return -1;
    if (y==x and y%2==1) return -1;
    if (y%2==1) return y-1;
    return y;
}
