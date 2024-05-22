#include<stdio.h>
#include<string>
#include<openssl/md5.h>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
string string_to_md5(string text){
    unsigned char md[16];
    if (text.length()==0) return "None";
    MD5_CTX c;
    int i;
   MD5_Init(&c);
   MD5_Update(&c, (unsigned char*)text.c_str(), text.length());
    MD5_Final(md, &c);
    string out_str="";
    for (int i=0;i<16;i++)
        {
            char w;
            if (md[i]<160) w=48+md[i]/16;
            else w=87+md[i]/16;
            out_str=out_str+w;
            if (md[i]%16<10) w=48+md[i]%16;
            else w=87+md[i]%16;
            out_str=out_str+w;
        }
    return out_str;
}
