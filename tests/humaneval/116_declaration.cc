#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> sort_array(vector<int> arr){
    vector<int> bin={};
    int m;

    for (int i=0;i<arr.size();i++)
    {
        int b=0,n=abs(arr[i]);
        while (n>0)
        {
            b+=n%2;n=n/2;
        }
        bin.push_back(b);
    }
    for (int i=0;i<arr.size();i++)
    for (int j=1;j<arr.size();j++)
    if (bin[j]<bin[j-1] or (bin[j]==bin[j-1] and arr[j]<arr[j-1]))
    {
        m=arr[j];arr[j]=arr[j-1];arr[j-1]=m;
        m=bin[j];bin[j]=bin[j-1];bin[j-1]=m;
    }
    return arr;
}
