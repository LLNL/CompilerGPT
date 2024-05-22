#include<stdio.h>
#include<string>
#include<algorithm>
using namespace std;
#include<math.h>
#include<stdlib.h>
string file_name_check(string file_name){
    int numdigit=0,numdot=0;
    if (file_name.length()<5) return "No";
    char w=file_name[0];
    if (w<65 or (w>90 and w<97) or w>122) return "No";
    string last=file_name.substr(file_name.length()-4,4);
    if (last!=".txt" and last!=".exe" and last!=".dll") return "No";
    for (int i=0;i<file_name.length();i++)
    {
        if (file_name[i]>=48 and file_name[i]<=57) numdigit+=1;
        if (file_name[i]=='.') numdot+=1;
    }
    if (numdigit>3 or numdot!=1) return "No";
    return "Yes"; 
}
