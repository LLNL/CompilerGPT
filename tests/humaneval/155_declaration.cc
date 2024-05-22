#include<stdio.h>
#include<math.h>
#include<string>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> even_odd_count(int num){
    string w=to_string(abs(num));
    int n1=0,n2=0;
    for (int i=0;i<w.length();i++)
    if (w[i]%2==1) n1+=1;
    else n2+=1;
    return {n2,n1};
}
