#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string change_base(int x,int base){
    string out="";
    while (x>0)
    {
        out=to_string(x%base)+out;
        x=x/base;
    }
    return out;
}
