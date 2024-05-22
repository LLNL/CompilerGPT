#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int do_algebra(vector<string> operato, vector<int> operand){
    vector<int> num={};
    vector<int> posto={};
    for (int i=0;i<operand.size();i++)
        posto.push_back(i);
    for (int i=0;i<operato.size();i++)
        if (operato[i]=="**") 
        {
        while (posto[posto[i]]!=posto[i]) posto[i]=posto[posto[i]];
        while (posto[posto[i+1]]!=posto[i+1]) posto[i+1]=posto[posto[i+1]];
        operand[posto[i]]=pow(operand[posto[i]],operand[posto[i+1]]);
        posto[i+1]=posto[i];
        }
    for (int i=0;i<operato.size();i++)
        if (operato[i]=="*" or operato[i]=="//") 
        {
        while (posto[posto[i]]!=posto[i]) posto[i]=posto[posto[i]];
        while (posto[posto[i+1]]!=posto[i+1]) posto[i+1]=posto[posto[i+1]];
        if (operato[i]=="*")
            operand[posto[i]]=operand[posto[i]]*operand[posto[i+1]];
        else
            operand[posto[i]]=operand[posto[i]]/operand[posto[i+1]];
        posto[i+1]=posto[i];
        }
    for (int i=0;i<operato.size();i++)
        if (operato[i]=="+" or operato[i]=="-") 
        {
        while (posto[posto[i]]!=posto[i]) posto[i]=posto[posto[i]];
        while (posto[posto[i+1]]!=posto[i+1]) posto[i+1]=posto[posto[i+1]];
        if (operato[i]=="+")
            operand[posto[i]]=operand[posto[i]]+operand[posto[i+1]];
        else
            operand[posto[i]]=operand[posto[i]]-operand[posto[i+1]];
        posto[i+1]=posto[i];
        }
    return operand[0];

}
