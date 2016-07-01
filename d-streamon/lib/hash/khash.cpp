#include <khash.hpp>
#include <iostream>



namespace bm {

//
// CONSTRUCTOR
//
KHASH::KHASH(HASH* _hash, unsigned int _nhash, unsigned int _shash, bool _norep):
    hash(_hash),hashdiglen(hash->getDigestLength()),nhash(_nhash), shash(_shash), norep(_norep),res (new unsigned[_nhash])
{

	// for simplicity, using unsigned as maximum digest size
	// consequence: no more than sizeof(unsigned int)
        //maximum digest size < hash function digest size
    assert(_shash<=sizeof(unsigned int)*8 && _shash<=_hash->getDigestLength()*8 && shash);

	// assert that infinite cycle because of insufficient number
	// of hash will not occur
    assert((_norep==1)?(_nhash <= (unsigned(1) << _shash)):1);

    unsigned int bitneeded=((shash*nhash));
    hashtodo=bitneeded/(hashdiglen<<3);
    if (hashtodo*bitneeded!=(hashdiglen<<3))
        hashtodo++;
    
    msgdig=new unsigned char [hashtodo*hashdiglen+4];
    memset((void*)msgdig,0,hashtodo*hashdiglen+4);

}

//
// COPY CONSTRUCTOR
//
KHASH::KHASH(const KHASH& copy):
    hash(copy.hash),hashdiglen(copy.hashdiglen),nhash(copy.nhash), shash(copy.shash), norep(copy.norep),res (new unsigned[copy.nhash]),hashtodo(hashtodo),
        msgdig(new unsigned char [hashtodo*hashdiglen+4])
{
    memcpy(res,copy.res,nhash<<3);
    memcpy(msgdig,copy.msgdig,hashtodo*hashdiglen+4);
    

}


KHASH & KHASH::operator =(const KHASH& copy)
{
    delete []res;
    delete []msgdig;
    hash=copy.hash;
    hashdiglen=copy.hashdiglen;
    hashtodo=copy.hashtodo;
    nhash=copy.nhash;
    shash=copy.shash;
    norep=copy.norep;
    res = new unsigned[copy.nhash];
    msgdig = new unsigned char [hashtodo*hashdiglen+4];

    memcpy(res,copy.res,nhash<<3);
    memcpy(msgdig,copy.msgdig,hashtodo*hashdiglen+4);

    return *this;
}


//
// DESTRUCTOR
//
KHASH::~KHASH()
{
    delete [] res;
    delete [] msgdig;
}



//
// MAIN FUNCTION: COMPUTE K DIGEST
//
void KHASH::compute(unsigned char *in, int len)
{
    //assert(len>0);

    //unsigned char in2[len];
    

	// compute first SHA hash; should be all needed most of the time;
	// more SHA are always needed when shash*nhash>160; more SHA MAY be
	// needed when no repetition is required... in this case this
	// depends on the input
    //memset((void*)msgdig,0,hashtodo*hashdiglen+4);
    hash->compute(in,len,msgdig);
    for(unsigned int i = 1; i<hashtodo;i++)
        hash->compute(msgdig+(i-1)*hashdiglen,len,msgdig+i*hashdiglen);
    

    //memset((void*)res,0,nhash*sizeof(unsigned int));
    
    unsigned int startoffset=0;
    unsigned int endoffset=shash;
    unsigned int bitoffset;
    unsigned int mask=0xFFFFFFFF >> (32 - shash);

    for (unsigned int i=0; i<nhash; ++i)
        {
            //recompute a new hash if we need more bits
            /*if (endoffset>(hash->getDigestLength()<<3))
            {
                memcpy((void*)in2,(void*)msgdig,hash->getDigestLength());
                //in = new unsigned char[hash->getDigestLength()];
                //for(unsigned int j = 0; j < hash->getDigestLength(); ++j)
                //    in[j]=msgdig[j];
                hash->compute(in2,len,msgdig);
                --i;
                startoffset=0;
                endoffset=shash;
                continue;
            }*/

            //copy 32 bits at one time
            res[i]=*(unsigned int*)(msgdig + (startoffset >> 3));
            bitoffset = startoffset & 7;
            res[i] = res[i] >> (bitoffset);
            if ((shash + bitoffset)>(sizeof(unsigned)<<3))
                res[i] = res[i] | ((msgdig[endoffset >> 3]) << (shash - (endoffset & 7)));
            res[i] = res[i] & mask;
            /*if (norep)
                for (unsigned j = 0; j < i; ++j)
                    if (res[i]==res[j])
                    {
                        res[i]=0;
                        --i;
                        break;
                    }*/
            startoffset +=shash;
            endoffset +=shash;
    }
    

}

}
