#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int smallest_change(vector<int> arr){
    int out=0;
    for (int i=0;i<arr.size()-1-i;i++)
        if (arr[i]!=arr[arr.size()-1-i])
            out+=1;
    return out;
}
