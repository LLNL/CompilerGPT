#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
bool valid_date(string date){
    int mm,dd,yy,i;
    if (date.length()!=10) return false;
    for (int i=0;i<10;i++)
        if (i==2 or i==5)
        {
            if (date[i]!='-') return false;
        }
        else
            if (date[i]<48 or date[i]>57) return false;

    mm=atoi(date.substr(0,2).c_str());
    dd=atoi(date.substr(3,2).c_str());
    yy=atoi(date.substr(6,4).c_str());
    if (mm<1 or mm>12) return false;
    if (dd<1 or dd>31) return false;
    if (dd==31 and (mm==4 or mm==6 or mm==9 or mm==11 or mm==2)) return false;
    if (dd==30 and mm==2) return false;
    return true;

}
