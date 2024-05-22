#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
int is_bored(string S){
    bool isstart=true;
    bool isi=false;
    int sum=0;
    for (int i=0;i<S.length();i++)
    {
        if (S[i]==' ' and isi) {isi=false; sum+=1;}
        if (S[i]=='I' and isstart) {isi=true;  }
        else isi=false;   
        if (S[i]!=' ') { isstart=false;}
        if (S[i]=='.' or S[i]=='?' or S[i]=='!') isstart=true;
    }
    return sum;
}
