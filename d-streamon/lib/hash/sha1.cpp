#include<sha1.hpp>
#include"./C/sha1.h"


namespace bm{

void SHA1::compute(const unsigned char key [] , int len, unsigned char output[] )
{
    sha1(key,len,output);
}

void SHA1::computeFile(const char* path, unsigned char output[] )
{
    sha1_file(path,output);
}

void SHA1::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    sha1_hmac(key,keylen,input,ilen,output);
}
    
}


