#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> parse_music(string music_string){ 
    string current="";
    vector<int> out={};
    if (music_string.length()>0)
        music_string=music_string+' ';
    for (int i=0;i<music_string.length();i++)
    {
        if (music_string[i]==' ')
        {
            if (current=="o") out.push_back(4);
            if (current=="o|") out.push_back(2);
            if (current==".|") out.push_back(1);
            current="";
        }
        else current+=music_string[i];
    }
    return out;
}
