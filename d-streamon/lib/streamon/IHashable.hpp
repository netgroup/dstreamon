
/* ###########################################
 *
 * IHASHABLE
 *
 * A simple interface that offers GetHash()
 * method that returns a Digest
 * 
 * The class who inherit this one,
 * need to implement his method
 *
 * 
 * Author: Giulio Picierro, CNIT
 *
 * Mail: giulio.picierro@uniroma2.it
 *
 * Version: 1.0
 *
 * ##########################################
 */


#pragma once

#include "Digest.hpp"


class IHashable
{
public:

    virtual Digest GetHash() const = 0;

};

