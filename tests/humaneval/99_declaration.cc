#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int closest_integer(string value){
    double w;
    w=atof(value.c_str());
    return round(w);
}
