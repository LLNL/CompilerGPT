#include<stdio.h>
#include<math.h>
#include<string>
#include<map>
using namespace std;
#include<algorithm>
#include<stdlib.h>
string sort_numbers(string numbers){
    map<string,int> tonum={{"zero",0},{"one",1},{"two",2},{"three",3},{"four",4},{"five",5},{"six",6},{"seven",7},{"eight",8},{"nine",9}};
    map<int,string> numto={{0,"zero"},{1,"one"},{2,"two"},{3,"three"},{4,"four"},{5,"five"},{6,"six"},{7,"seven"},{8,"eight"},{9,"nine"}};
    int count[10];
    for (int i=0;i<10;i++)
        count[i]=0;
    string out="",current="";
    if (numbers.length()>0) numbers=numbers+' ';
    for (int i=0;i<numbers.length();i++)
        if (numbers[i]==' ')
        {
            count[tonum[current]]+=1;
            current="";
        }
        else current+=numbers[i];
    for (int i=0;i<10;i++)
        for (int j=0;j<count[i];j++)
            out=out+numto[i]+' ';
    if (out.length()>0) out.pop_back();
    return out;
}
