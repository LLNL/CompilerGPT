#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool move_one_ball(vector<int> arr){
    int num=0;
    if (arr.size()==0) return true;
    for (int i=1;i<arr.size();i++)
        if (arr[i]<arr[i-1]) num+=1;
    if (arr[arr.size()-1]>arr[0]) num+=1;
    if (num<2) return true;
    return false;
}
