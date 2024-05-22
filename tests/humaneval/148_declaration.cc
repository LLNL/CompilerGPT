#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<string> bf(string planet1,string planet2){
    vector<string> planets={"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};
    int pos1=-1,pos2=-1,m;
    for (m=0;m<planets.size();m++)
    {
    if (planets[m]==planet1) pos1=m;
    if (planets[m]==planet2) pos2=m;
    }
    if (pos1==-1 or pos2==-1) return {};
    if (pos1>pos2) {m=pos1;pos1=pos2;pos2=m;}
    vector<string> out={};
    for (m=pos1+1;m<pos2;m++)
    out.push_back(planets[m]);
    return out;
}
