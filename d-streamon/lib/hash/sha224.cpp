#include<sha224.hpp>
#include"./C/sha2.h"


namespace bm{

void SHA224::compute(const unsigned char key [] , int len, unsigned char output [] )
{
    sha2(key,len,output,1);
}

void SHA224::computeFile(const char* path, unsigned char output [])
{
    sha2_file(path,output,1);
}

void SHA224::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    sha2_hmac(key,keylen,input,ilen,output,1);
}

}

