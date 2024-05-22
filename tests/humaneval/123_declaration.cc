#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> get_odd_collatz(int n){
    vector<int> out={1};
    while (n!=1)
    {
        if (n%2==1) {out.push_back(n); n=n*3+1;}
        else n=n/2;
    }
    sort(out.begin(),out.end());
    return out;
}
