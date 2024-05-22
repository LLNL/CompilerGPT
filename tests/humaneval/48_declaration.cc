#include<stdio.h>
#include<math.h>
#include<string>
using namespace std;
#include<algorithm>
#include<stdlib.h>
bool is_palindrome(string text){
    string pr(text.rbegin(),text.rend());
    return pr==text;
}
