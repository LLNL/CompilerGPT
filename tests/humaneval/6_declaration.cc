#include<stdio.h>
#include<vector>
#include<string>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
vector<int> parse_nested_parens(string paren_string){
    vector<int> all_levels;
    string current_paren;
    int level=0,max_level=0;
    char chr;
    int i;
    for (i=0;i<paren_string.length();i++)
    {
        chr=paren_string[i];
        if (chr=='(')
        {
        level+=1;
        if (level>max_level) max_level=level;
        current_paren+=chr;
        }
        if (chr==')')
        {
            level-=1;
            current_paren+=chr;
            if (level==0){
                all_levels.push_back(max_level);
                current_paren="";
                max_level=0;
            }
        }
    }
    return all_levels;
}
