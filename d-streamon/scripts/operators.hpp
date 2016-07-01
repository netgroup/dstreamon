/***************************************
 * OPERATORS.HPP                       *
 *                                     *
 * This file will store operators      *
 * needed by selector engine to        *
 * select packets                      *
 *                                     *
 * Author: Giulio Picierro, CNIT       *
 * Email: giulio.picierro@uniroma2.it  *
 *                                     *
 ***************************************/


#pragma once

#include <streamon/Streamon.hpp>

// EQUALITY OPERATORS

template<typename T>
bool operator==(const IReadBuffer& buff, T elem)
{
    if (buff.GetLength() != sizeof(elem)) return false;

    return memcmp( buff.GetRawData(), &elem, sizeof(elem) ) == 0;
}

bool operator==(const IReadBuffer& buff, std::string& s)
{
    if (buff.GetLength() != s.size()) return false;

    return strncmp( (const char*) buff.GetRawData(), s.c_str(), buff.GetLength() ) == 0;
}


// LESS-THAN OPERATOR

template<typename T>
bool operator<(const IReadBuffer& buff, T elem)
{
    if (buff.GetLength() != sizeof(elem)) return false;

    /*
    auto b = buff.GetRawData();

    for (int i=3; i >= 0; i--)
        printf("%02x", b[i]);

    std::cout << " - ";

    auto v = (uint8_t*) &elem;

    for (int i=3; i >= 0; i--)
        printf("%02x", v[i]);

    std::cout << " -- ";


    std::cout << buff.GetLength() << " -- " << *b << " <? " << elem << " " << (memcmp(b,v,4)) << "\n";
    */

    auto temp = *(T*) buff.GetRawData();

    return temp < elem; // memcmp( buff.GetRawData(), &elem, sizeof(elem) ) < 0;
}


bool operator<(const IReadBuffer& buff, std::string& s)
{
    if (buff.GetLength() != s.size()) return false;

    return strncmp( (const char*) buff.GetRawData(), s.c_str(), buff.GetLength() ) < 0;

}


// GREATER-THAN OPERATOR

template<typename T>
bool operator>(const IReadBuffer& buff, T elem)
{
    if (buff.GetLength() != sizeof(elem)) return false;

    auto temp = *(T*) buff.GetRawData();

    return temp > elem; // memcmp( buff.GetRawData(), &elem, sizeof(elem) ) > 0;
}


bool operator>(const IReadBuffer& buff, std::string& s)
{
    if (buff.GetLength() != s.size()) return false;

    return strncmp( (const char*) buff.GetRawData(), s.c_str(), buff.GetLength() ) > 0;

}


// OTHER OPERATORS

template<typename T>
bool operator!=(const IReadBuffer& buff, T elem) { return !operator==(buff, elem); }

template<typename T>
bool operator>=(const IReadBuffer& buff, T elem) { return operator>(buff, elem) || operator==(buff, elem); }

template<typename T>
bool operator<=(const IReadBuffer& buff, T elem) { return operator<(buff, elem) || operator==(buff, elem); }


// FUNCTIONAL OPERATORS

template<typename T>
bool in(const IReadBuffer& buff)
{
    return false;
}
