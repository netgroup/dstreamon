/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_FACTORY_HPP_
#define _MORE_FACTORY_HPP_ 

#include <type_traits>
#include <memory>
#include <string>
#include <map>

namespace more { 

    template <typename B, typename ... Arg>  
    struct factory_base_allocator
    {
        virtual ~factory_base_allocator() 
        {}

        virtual B * alloc(Arg&& ... ) = 0;
        virtual std::shared_ptr<B> shared_alloc(Arg&& ... ) = 0;
    };

    template <typename B, typename E, typename ... Arg>   // B must be a base class of E 
    struct factory_allocator : public factory_base_allocator<B,Arg...>
    {
        static_assert(std::is_base_of<B,E>::value, "base_of relationship violated");

        virtual B * alloc(Arg && ... arg)
        {
            return new E(std::forward<Arg>(arg)...);
        }

        virtual std::shared_ptr<B> shared_alloc(Arg && ... arg)
        {
            return std::make_shared<E>(std::forward<Arg>(arg)...);
        }
    };

    /////////////////////////////////////////////////////////////////////
    // utility: auto-register the allocator of E element (derived from B)
    //                to the F factory

    template <typename ... Tp>
    struct fac_args {};

    template <typename B  /* Base */,  
              typename E  /* element */
              >
    struct factory_register
    {
        static_assert(std::is_base_of<B,E>::value, "base_of relationship violated");

        template <typename F, typename K>
        factory_register(F &f, const K &key)
        {
            unpack_register(f, key, typename F::args_pack());
        }

        template <typename F, typename K, typename ... Ti>
        void unpack_register(F &f, const K &key, fac_args<Ti...>)
        {
            f.regist(key, std::unique_ptr<factory_base_allocator<B, Ti...>>(new 
                                            typename more::factory_allocator<B, E, Ti...>()));    
        }
    };

    /////////////////////////////////////////
    // factory class: K:key -> B:base_element


    template <typename K, typename B, typename ... Arg> 
    class factory
    {
    public:
        
#if __GNUC__ == 4 && __GNUC_MINOR__ < 6
        // buggy std::map when used with std::unique_ptr...    
        //
        typedef std::map<K, std::shared_ptr<factory_base_allocator<B,Arg...>>> map_type;
#else
        typedef std::map<K, std::unique_ptr<factory_base_allocator<B,Arg...>>> map_type;
#endif

        typedef fac_args<Arg...> args_pack;

        factory()  = default;
        ~factory() = default;

        bool
        regist(const K & key, std::unique_ptr<factory_base_allocator<B, Arg...>> value)
        { 
#if __GNUC__ == 4 && __GNUC_MINOR__ < 6
            std::shared_ptr<factory_base_allocator<B, Arg...>> sp(std::move(value));
            return m_map.insert(make_pair(key, std::move(sp))).second; 
#else                                                    
            return m_map.insert(make_pair(key, std::move(value))).second; 
#endif
        }
        
        bool
        unregist(const K &key)
        { 
            return m_map.erase(key) == 1; 
        }

        bool
        is_registered(const K &key) const
        {
            return m_map.count(key) != 0;
        }

        template <typename ...Ti>
        std::unique_ptr<B> 
        operator()(const K &key, Ti&& ... arg) const
        {
            auto it = m_map.find(key);
            if (it == m_map.end())
                return std::unique_ptr<B>();

            return std::unique_ptr<B>(it->second->alloc(std::forward<Ti>(arg)...));
        }

        template <typename ...Ti>
        std::shared_ptr<B> 
        shared(const K &key, Ti&& ... arg) const
        {
            auto it = m_map.find(key);
            if (it == m_map.end())
                return std::shared_ptr<B>();

            return it->second->shared_alloc(std::forward<Ti>(arg)...);
        }

    private:
        map_type m_map;
    };

} // namespace more

#endif /* _MORE_FACTORY_HPP_ */
