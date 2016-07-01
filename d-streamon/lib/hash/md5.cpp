#include<md5.hpp>
#include"./C/md5.h"


namespace bm{

void MD5::compute(const unsigned char key [] , int len, unsigned char output [] )
{
    md5(key,len,output);
}

void MD5::computeFile(const char* path, unsigned char output [])
{
    md5_file(path,output);
}

void MD5::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    md5_hmac(key,keylen,input,ilen,output);
}

}

