#include<stdio.h>
#include<math.h>
#include<string>
#include<algorithm>
using namespace std;
#include<stdlib.h>
bool check_if_last_char_is_a_letter(string txt){
    if (txt.length()==0) return false;
    char chr=txt[txt.length()-1];
    if (chr<65 or (chr>90 and chr<97) or chr>122) return false;
    if (txt.length()==1) return true;
    chr=txt[txt.length()-2];
    if ((chr>=65 and chr<=90) or (chr>=97 and chr<=122)) return false;
    return true;
}
