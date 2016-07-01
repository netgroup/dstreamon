#include<sha256.hpp>
#include"./C/sha2.h"


namespace bm{

void SHA256::compute(const unsigned char key [] , int len, unsigned char output [] )
{
    sha2(key,len,output,0);
}

void SHA256::computeFile(const char* path, unsigned char output [])
{
    sha2_file(path,output,0);
}

void SHA256::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    sha2_hmac(key,keylen,input,ilen,output,0);
}

}

