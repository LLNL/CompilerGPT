#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int skjkasdkd(vector<int> lst){
    int largest=0;
    for (int i=0;i<lst.size();i++)
        if (lst[i]>largest)
        {
            bool prime=true;
            for (int j=2;j*j<=lst[i];j++)
                if (lst[i]%j==0) prime=false;
            if (prime) largest=lst[i];
        }
    int sum=0;
    string s;
    s=to_string(largest);
    for (int i=0;i<s.length();i++)
        sum+=s[i]-48;
    return sum;
}
#undef NDEBUG
#include<assert.h>
