#include<sha384.hpp>
#include"./C/sha4.h"


namespace bm{

void SHA384::compute(const unsigned char key [] , int len, unsigned char output [] )
{
    sha4(key,len,output,1);
}

void SHA384::computeFile(const char* path, unsigned char output [])
{
    sha4_file(path,output,1);
}

void SHA384::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    sha4_hmac(key,keylen,input,ilen,output,1);
}

}


