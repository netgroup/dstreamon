#include <ACHash31.hpp>
#include <fstream>


namespace bm{

unsigned int ACHash31::compute(unsigned int key)
{
    return compute(key,SMOD_A,SMOD_B);
}

unsigned int ACHash31::compute(unsigned int key, unsigned int a, unsigned int b)
{
    unsigned long long result;
    result = ((unsigned long long) a)*((unsigned long long) key)+((unsigned long long) b);
    result = ((result >> 31) + result) & SMOD_MOD;
    return (unsigned int) result;
}

void ACHash31::compute(const unsigned char key [] , int len, unsigned char output[] )
{
    unsigned int key_int = 0;
    unsigned int key_int_temp = 0;
    for (int i = 0; i < len; i++)
    {
        if((i&0x3)==0)
        {
            key_int += key_int_temp;
            key_int_temp = 0;
        }
        key_int_temp += key[i] << ((i&0x3)<<3); //(i%4)*8
    }
    key_int +=key_int_temp;
    unsigned int out = compute(key_int);
    for (unsigned int i = 0; i < getDigestLength();i++)
    {
        output[i]=out & 0xFF;
        out = out>>8;
    }
}

void ACHash31::computeFile(const char* path, unsigned char output[] )
{
    std::ifstream input;
    input.open(path);
    unsigned int tmp = 0;
    unsigned int key = 0;
    while (!input.eof())
    {
        input >> tmp;
        key += tmp;
    }
    unsigned char* ckey = (unsigned char*)&key;
    compute(ckey,4,output);
}

void ACHash31::compute(const unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[] )
{
    unsigned int key_int = 0;
    unsigned int key_int_temp = 0;

    //prepare the key
    for (int i = 0; i < keylen; i++)
    {
        if((i&0x3)==0)
        {
            key_int += key_int_temp;
            key_int_temp = 0;
        }
        key_int_temp += key[i] << ((i&0x3)<<3); //(i%4)*8
    }
    key_int +=key_int_temp;
    key_int_temp = 0;
    //add the input message
    for (int i = 0; i < ilen; i++)
    {
        if((i&0x3)==0)
        {
            key_int += key_int_temp;
            key_int_temp = 0;
        }
        key_int_temp += input[i] << ((i&0x3)<<3); //(i%4)*8
    }
    key_int +=key_int_temp;
    unsigned int out = compute(key_int);
    for (unsigned int i = 0; i < getDigestLength();i++)
    {
        output[i]=out & 0xFF;
        out = out>>8;
    }
}

}

