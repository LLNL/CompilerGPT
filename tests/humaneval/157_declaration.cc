#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool right_angle_triangle(float a,float b,float c){
    if (abs(a*a+b*b-c*c)<1e-4 or abs(a*a+c*c-b*b)<1e-4 or abs(b*b+c*c-a*a)<1e-4) return true;
    return false;
}
