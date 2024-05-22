#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> even_odd_palindrome(int n){
    int num1=0,num2=0;
    for (int i=1;i<=n;i++)
    {
        string w=to_string(i);
        string p(w.rbegin(),w.rend());
        if (w==p and i%2==1) num1+=1;
        if (w==p and i%2==0) num2+=1;
            
    }
    return {num2,num1};
}
