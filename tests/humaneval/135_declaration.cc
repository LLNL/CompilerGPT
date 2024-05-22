#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
int can_arrange(vector<int> arr){
    int max=-1;
    for (int i=0;i<arr.size();i++)
    if (arr[i]<=i) max=i;
    return max;
}
