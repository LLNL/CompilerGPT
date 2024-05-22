#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> sort_array(vector<int> array){
    if (array.size()==0) return {};
    if ((array[0]+array[array.size()-1]) %2==1)
    {
        sort(array.begin(),array.end());
        return array;
    }
    else
    {
        sort(array.begin(),array.end());
        vector<int> out={};
        for (int i=array.size()-1;i>=0;i-=1)
            out.push_back(array[i]);
        return out;
    }

}
