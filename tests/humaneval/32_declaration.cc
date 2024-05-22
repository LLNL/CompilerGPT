#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
double poly(vector<double> xs, double x){
    double sum=0;
    int i;
    for (i=0;i<xs.size();i++)
    {
        sum+=xs[i]*pow(x,i);
    }
    return sum;
}

double find_zero(vector<double> xs){
    double ans=0;
    double value;
    value=poly(xs,ans);
    while (abs(value)>1e-6)
    {
        double driv=0;
        for (int i=1;i<xs.size();i++)
        {
            driv+=xs[i]*pow(ans,i-1)*i;
        }
        ans=ans-value/driv;
        value=poly(xs,ans);
    }
    return ans;

}
