#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
long long minSubArraySum(vector<long long> nums){
    long long current,min;
    current=nums[0];
    min=nums[0];
    for (int i=1;i<nums.size();i++)
    {
       if (current<0) current=current+nums[i];
       else current=nums[i];
       if (current<min) min=current;
    }
    return min;
}
