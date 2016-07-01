#include "bloompair.h"


using namespace std;

//
// CONSTRUCTOR
//
BloomPair::BloomPair(int _nhash, int _shash, BloomPair** _slave_place) : khash(_nhash, _shash, 1) {
	
	// contruct the two bloom filters
    B_learning = new bloomfilter(_nhash, _shash, 1);
    B_detecting = new bloomfilter(_nhash, _shash, 1);

	// and set the basic parameters and initial threshold
    m = B_learning->maxdigest();
    assert(m==B_learning->get_nzero());
    threshold = double(m)/sqrt(2.0);

    slave_place = _slave_place;

	lock = Thread_lock::init_lock();
}

BloomPair::~BloomPair() {
    delete B_learning;
    delete B_detecting;
    }


void BloomPair::clear()
{
	lock->lock(true);

    B_learning->clear();
    B_detecting->clear();
    threshold = double(m)/sqrt(2.0);

	lock->unlock();
}
	

bool BloomPair::add(unsigned char *in, int len) {
	// update filters
    bool res_l = B_learning->insert(in, len);
    bool res_d = B_detecting->insert(in, len);
    assert((res_d==false)||((res_d==true)&&(res_l==true)));
    // double dd = double(B_detecting->get_nzero()); /* NOW COMPUTED IN SWAP */
    // double dl = double();

	// eventually swap
    if (B_learning->get_nzero()<threshold) { 
      swap();
    }

	// debug / plot print
/*
    fprintf(fp,"%.0f ", dl);
    fprintf(fp,"%.0f ", dd);
    fprintf(fp,"%.0f ", threshold);
    char t = in[0];
    if (t=='R') fprintf(fp, "%d ", (res_d==true)?10:0);
    if (t=='V') fprintf(fp, "%d ", (res_d==false)?-10:0);
    fprintf(fp,"%s ", in);
    //fprintf(fp,"%s ", (res_l==true)?"LRN-Y":"LRN-N");
    //fprintf(fp,"%s ", (res_d==true)?"DET-Y":"DET-N");
    fprintf(fp,"\n");
*/

    return res_d;
    }

bool BloomPair::check(unsigned char* element, int len) {

	return B_detecting->check(element, len);
}



bool BloomPair::set_bits(const unsigned* bits, int len)
{
	lock->lock(true);

	// update filters
	bool res_l = B_learning->set_bits(bits, len);
	bool res_d = B_detecting->set_bits(bits, len);
	assert((res_d==false)||((res_d==true)&&(res_l==true)));
	// double dd = double(B_detecting->get_nzero());

	// double dl = double(B_learning->get_nzero());
  
	// eventually swap
	if (B_learning->get_nzero()<threshold) { 
		swap();
	}

	lock->unlock();
  
	return res_d;
}


bool BloomPair::check_bits(const unsigned* bits, int len)
{
	lock->lock(false);

	bool res = B_detecting->check_bits(bits, len);

	lock->unlock();

	return res;
}
