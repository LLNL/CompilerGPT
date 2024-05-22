#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int add_elements(vector<int> arr,int k){
    int sum=0;
    for (int i=0;i<k;i++)
        if( arr[i]>=-99 and arr[i]<=99)
            sum+=arr[i];
    return sum;
}
