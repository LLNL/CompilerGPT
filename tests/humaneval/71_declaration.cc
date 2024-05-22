#include<stdio.h>
#include<math.h>
using namespace std;
#include<algorithm>
#include<stdlib.h>
float triangle_area(float a,float b,float c){
    if (a+b<=c or a+c<=b or b+c<=a) return -1;
    float h=(a+b+c)/2;
    float area;
    area=pow(h*(h-a)*(h-b)*(h-c),0.5);
    return area;
}
