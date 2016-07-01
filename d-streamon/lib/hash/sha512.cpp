#include<sha512.hpp>
#include"./C/sha4.h"


namespace bm{

void SHA512::compute(const unsigned char key [] , int len, unsigned char output [] )
{
    sha4(key,len,output,0);
}

void SHA512::computeFile(const char* path, unsigned char output [])
{
    sha4_file(path,output,0);
}

void SHA512::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    sha4_hmac(key,keylen,input,ilen,output,0);
}

}
