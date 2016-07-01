#include<bob.hpp>
#include<iostream>
#include<fstream>
#include"./C/bob.c"
#include<string.h>

namespace bm{

void BOB::compute(const unsigned char key [] , int len, unsigned char output [] )
{
    *(unsigned int*)output=hashlittle((void*)key,len,INITNUMB);
}

void BOB::computeFile(const char* path, unsigned char output [])
{
    std::ifstream::pos_type size;
    char * memblock;
    std::ifstream file (path, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (memblock, size);
        file.close();
        *(unsigned int*)output=hashlittle((void*)memblock,size,INITNUMB);
        delete[] memblock;
    }

}

void BOB::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    unsigned char* keyin = new unsigned char[keylen+ilen];
    memcpy(keyin,key,keylen);
    memcpy(keyin+keylen,input,ilen);
    *(unsigned int*)output=hashlittle((void*)keyin,ilen+keylen,INITNUMB);
    delete [] keyin;
}

}

