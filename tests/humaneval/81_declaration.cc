#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<string> numerical_letter_grade(vector<float> grades){
    vector<string> out={};
    for (int i=0;i<grades.size();i++)
    {
        if (grades[i]>=3.9999) out.push_back("A+");
        if (grades[i]>3.7001 and grades[i]<3.9999) out.push_back("A");
        if (grades[i]>3.3001 and grades[i]<=3.7001) out.push_back("A-");
        if (grades[i]>3.0001 and grades[i]<=3.3001) out.push_back("B+");
        if (grades[i]>2.7001 and grades[i]<=3.0001) out.push_back("B");
        if (grades[i]>2.3001 and grades[i]<=2.7001) out.push_back("B-");
        if (grades[i]>2.0001 and grades[i]<=2.3001) out.push_back("C+");
        if (grades[i]>1.7001 and grades[i]<=2.0001) out.push_back("C");
        if (grades[i]>1.3001 and grades[i]<=1.7001) out.push_back("C-");
        if (grades[i]>1.0001 and grades[i]<=1.3001) out.push_back("D+");
        if (grades[i]>0.7001 and grades[i]<=1.0001) out.push_back("D");
        if (grades[i]>0.0001 and grades[i]<=0.7001) out.push_back("D-");
        if (grades[i]<=0.0001) out.push_back("E");
    }
    return out;
}
