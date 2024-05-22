#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> make_a_pile(int n){
    vector<int> out={n};
    for (int i=1;i<n;i++)
        out.push_back(out[out.size()-1]+2);
    return out;
}
