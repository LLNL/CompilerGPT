#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> order_by_points(vector<int> nums){
    vector<int> sumdigit={};
    for (int i=0;i<nums.size();i++)
    {
        string w=to_string(abs(nums[i]));
        int sum=0;
        for (int j=1;j<w.length();j++)
            sum+=w[j]-48;
        if (nums[i]>0) sum+=w[0]-48;
        else sum-=w[0]-48;
        sumdigit.push_back(sum);
    }
    int m;
    for (int i=0;i<nums.size();i++)
    for (int j=1;j<nums.size();j++)
    if (sumdigit[j-1]>sumdigit[j])
    {
        m=sumdigit[j];sumdigit[j]=sumdigit[j-1];sumdigit[j-1]=m;
        m=nums[j];nums[j]=nums[j-1];nums[j-1]=m;
    }
     
    return nums;
}
