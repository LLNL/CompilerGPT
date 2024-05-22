#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string circular_shift(int x,int shift){
    string xs;
    xs=to_string(x);
    if (xs.length()<shift)
    {
        string s(xs.rbegin(),xs.rend());
        return s;
    }
    xs=xs.substr(xs.length()-shift)+xs.substr(0,xs.length()-shift);
    return xs;
}
