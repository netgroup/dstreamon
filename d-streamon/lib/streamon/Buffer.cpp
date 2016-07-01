
/* ###########################################
 *
 * BUFFER<>
 *
 * A simple template class that allows handling 
 * of data types in an uniform way
 * (as a sequence of bytes) and also
 * allows concatenation of sequences of bytes
 *
 * 
 * Author: Giulio Picierro, CNIT
 *
 * Mail: giulio.picierro@uniroma2.it
 *
 * ##########################################
 */

#include <iostream>
#include <cstring>
#include <string>
#include <istream>
#include <sstream>
#include <stdexcept>

#include "IBuffer.hpp"

template <int S>
class Buffer : public IReadBuffer
{

protected:

  // buffer_type Type;

  unsigned char Data[S];


public:

    Buffer() : IReadBuffer()
    {
        static_assert( S <= 65536, "Maximum Buffer size is 65536 bytes." );
    }

  template<typename T>
  Buffer& append(T Val, buffer_type Ty = buffer_type::UNKNOWN)
  {
     return append( &Val, sizeof(Val), Ty );
  }


  virtual Buffer& append(const void* Ptr, int Len, buffer_type Ty = buffer_type::UNKNOWN)
  {
    if ( Count + Len > S ) throw std::out_of_range(__BUFFER_ERROR_STR);

    Type = Ty;

    memcpy( Data + Count, Ptr, Len );

    Count += Len;

    return *this;
  }

  virtual const unsigned char* GetRawData() const 
  {
    return Data;
  }




/*
  uint32_t GetUInt32(unsigned long Offset = 0) const
  {
    if ( Count - Offset < sizeof(uint32_t) )
    {
      std::cout << "GetUInt32(): Buffer len " << Count << ".\n";
      throw std::out_of_range(__BUFFER_ERROR_GET);
    }


    uint32_t Value = *(uint32_t*) (Data + Offset);

    return Value;
  }
*/

/*
  double GetDouble(unsigned long Offset = 0) const
  {
    if ( Count - Offset < sizeof(double) )
    {
      std::cout << "GetDouble(): Buffer len " << Count << ".\n";
      throw std::out_of_range(__BUFFER_ERROR_GET);
    }


    double Value = *(double*) (Data + Offset);

    return Value; 
  }
*/

  void Reset()
  {
    Count = 0;
  }


  virtual Digest GetHash() const
  {
     throw std::runtime_error("GetHash not implemented");
  }


  template<typename T>
  bool operator==(const T& value) const
  {
      if (Count != sizeof(value)) return false;

      return memcmp( Data, &value, Count ) == 0;
  }


  bool operator==(const IReadBuffer& other) const
  {
      if ( this == &other ) return true;
      else
      if (Count != other.Count) return false;

      return memcmp( Data, other.GetRawData(), Count ) == 0;
  }


  bool operator==(const Buffer& other) const
  {
      if ( this == &other ) return true;
      else
      if (Count != other.Count) return false;

      return memcmp( Data, other.Data, Count ) == 0;
  }


  /*
  bool operator==(const Buffer& Other) const
  {
    if ( this == &Other ) return true;
    else
    if (Count != Other.Count) return false;


    return memcmp( Data, Other.Data, Count ) == 0;
  }


  bool operator==(uint32_t Var) const
  {
    if ( Count == sizeof(Var) )
    {
      return memcmp( Data, &Var, Count ) == 0;
    }

    return false;
  }
  */


  Buffer& operator<<(const IReadBuffer& Var)
  {
    // Type = buffer_type::UNKNOWN;

    return append( Var.GetRawData(), Var.GetLength() );
  }

  
  Buffer& operator<<(const uint8_t Var)
  {
    uint32_t Val = Var;

    // Type = buffer_type::UINT;

    return append( &Val, sizeof(Val), buffer_type::UINT );
  }


  Buffer& operator<<(const uint16_t Var)
  {
    uint32_t Val = Var;

    // Type = buffer_type::UINT;

    return append( &Val, sizeof(Val), buffer_type::UINT );
  }


  Buffer& operator<<(const uint32_t Var)
  {
    // Type = buffer_type::UINT;

    return append( &Var, sizeof(Var), buffer_type::UINT );
  }


  Buffer& operator<<(const double Var)
  {
    // Type = buffer_type::DOUBLE;

    return append( &Var, sizeof(Var), buffer_type::DOUBLE );
  }


  Buffer& operator<<(const std::string& Var)
  {
    // Type = buffer_type::STRING;

    return append( Var.c_str(), Var.size(), buffer_type::STRING );
  }

};
