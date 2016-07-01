#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_ 

#include <vector>
#include <utility>
#include <string>

/**
 * This is a simple implementation of buffer class, inspired to that of boost::asio.
 * It provides a template version of both const_buffer and mutable_buffer. 
 *
 * The classes have no ownership of the memory and their use is intended to provide
 * a uniform IO interface for a bunch of different data: raw pointers, std::vector, 
 * std::basic_string etc. 
 *
 **/

namespace bm { 

    /**
     * forward declaration of const_buffer class;
     *
     */

    template <typename Tp>
    struct const_buffer;

    /**
     *  The mutable_buffer class provides a safe representation of a buffer of Tp objects 
     *  that can be modified. It does not own the underlying data, and so is cheap to copy or
     *  assign.
     */

    template <typename Tp>
    struct mutable_buffer
    {
        friend struct const_buffer<Tp>;

        /**
         * default constructor: create a null mutable_buffer 
         */

        mutable_buffer()
        : m_addr(0), m_len(0)
        {}

        /**
         * copy constructor
         *
         */

        mutable_buffer(const mutable_buffer &other)
        : m_addr(other.m_addr),
        m_len(other.m_len)
        {}

        /**
         * assignment operator
         *
         */

        mutable_buffer & operator=(const mutable_buffer &other)
        {
            m_addr = other.m_addr;
            m_len  = other.m_len;
            return *this;
        }

        /** Moveability is explicitly deleted for this class.
        */

        mutable_buffer(mutable_buffer &&other) = delete;
        mutable_buffer & operator=(mutable_buffer &&other) = delete;

        /**
         * constructor: create a mutable_buffer referencing to an external
         * contiguous memory of numb Tp objects.
         *
         * @param: addr Address of the first object
         * @param: numb Number of objects  
         */

        mutable_buffer(Tp *addr, size_t numb)
        : m_addr(addr), m_len(numb)
        {}

        /**
         * constructor: create a mutable_buffer referencing to an external
         * std::vector of Tp objects.
         *
         * @param: vec L-value reference of std::vector<Tp>
         */

        template <typename Alloc>
        mutable_buffer(std::vector<Tp, Alloc> &vec)
        : m_addr(&vec.front()), m_len(vec.size())
        {}

        /**
         * addr: return the pointer to the first
         * Tp object referenced by this mutable_buffer (const version)
         *
         */

        const Tp *
        addr() const
        {
            return m_addr;
        }

        /**
         * addr: return the pointer to the first Tp object referenced by 
         * this mutable_buffer 
         *
         */

        Tp *
        addr()
        {
            return m_addr;
        }

        /**
         * len: return the number of Tp objects * referenced by 
         * this mutable_buffer 
         *
         */

        size_t
        len() const
        {
            return m_len;
        }

    private:
        Tp       *m_addr;
        size_t    m_len; 
    };

    /**
     *  The const_buffer class provides a safe representation of a buffer of Tp objects 
     *  that cannot be modified. It does not own the underlying data, and so is cheap to copy or
     *  assign.
     */

    template <typename Tp>
    struct const_buffer
    {
        /**
         * default constructor: create a null const_buffer 
         */

        const_buffer()
        : m_addr(0), m_len(0)
        {}

        /**
         * copy constructor
         *
         */

        const_buffer(const const_buffer &other)
        : m_addr(other.m_addr),
        m_len(other.m_len)
        {}

        /**
         * assignment operator
         *
         */

        const_buffer & operator=(const const_buffer &other)
        {
            m_addr = other.m_addr;
            m_len  = other.m_len;
            return *this;
        }

        /** Moveability is explicitly deleted for this class.
        */

        const_buffer(const_buffer &&other) = delete;
        const_buffer & operator=(const_buffer &&other) = delete;

        /**
         * constructor: create a const_buffer referencing to an external
         * contiguous memory of numb const Tp objects.
         *
         * @param: addr Address of the first object
         * @param: numb Number of objects  
         */

        const_buffer(const Tp *addr, size_t numb)
        : m_addr(addr), m_len(numb)
        {}

        /**
         * constructor: create a const_buffer from a mutable_buffer
         * Note: viceversa is forbidden.
         *
         * @param: other A const reference to a compatible mutable_buffer
         */

        const_buffer(const mutable_buffer<Tp> &other)
        : m_addr(other.addr()), m_len(other.len())
        {}

        /**
         * constructor: create a const_buffer referencing to an external
         * std::vector of Tp objects.
         *
         * @param: vec L-value reference of std::vector<Tp>
         */

        template <typename Alloc>
        const_buffer(const std::vector<Tp, Alloc> &v)
        : m_addr(&v.front()), m_len(v.size())
        {}

        /**
         * constructor: create a const_buffer referencing to an external
         * std::basic_string<> (ie: std::string or std::wstring).
         *
         * @param: s The const reference to the std::basic_string
         */

        template <typename Traits, typename Alloc>
        const_buffer(const std::basic_string<Tp,Traits, Alloc> &s)
        : m_addr(s.data()), m_len(s.size())
        {}

        /**
         * addr: return the pointer to the first
         * Tp object referenced by this const_buffer 
         *
         */

        const Tp *
        addr() const
        {
            return m_addr;
        }

        /**
         * len: return the number of Tp objects 
         * referenced by this const_buffer 
         *
         */

        size_t
        len() const
        {
            return m_len;
        }

    private:
        const Tp *m_addr;
        size_t    m_len; 
    };

} // namespace bm

#endif /* _BUFFER_HPP_ */
