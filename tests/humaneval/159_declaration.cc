#include<stdio.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
vector<int> eat(int number,int need,int remaining){
    if (need>remaining) return {number+remaining, 0};
    return {number+need,remaining-need};
}
