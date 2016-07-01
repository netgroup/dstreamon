#ifndef _BLOCKFACTORY_HPP_
#define _BLOCKFACTORY_HPP_ 

/**
  * BlockFactory.hpp
  * Essentially a wrapper around the generic factory in the more library.
  * Standard c++ factory pattern
  */


#include <Factory.hpp>
#include <Block.hpp>
#include <string>

namespace bm
{

    /**
      * Singleton factory class (it simply wraps a more::factory object
      */

    class BlockFactory
    {
       
    public: 
        /**
          * Type of the factory object.
          * It is actually an instantiation of the more template factory class
          */
        typedef more::factory<std::string, bm::Block, std::string, bool> fac_type;

    private:
        fac_type m_actual_factory;

        /**
          * class constructor.
          * private as in Meyer's singleton
          */
        BlockFactory():
        m_actual_factory()
        {}

        ~BlockFactory()
        {}

        /**
          * the factory object is not copiable nor movable
          */
        BlockFactory(const BlockFactory &) = delete;
        BlockFactory& operator=(const BlockFactory &) = delete;
        BlockFactory( BlockFactory &&) = delete;
        BlockFactory& operator=( BlockFactory &&) = delete;
        

    public:
        /**
          * unique instance accessor as in Meyer's singleton
          * @return a reference to the only instance of this class
          */
        static BlockFactory &
        instance()
        {
            static BlockFactory the_factory;
            return the_factory;	
        }

        /**
          * In order for the underlying machinery to work, the registration class needs to access the internals
          */
        fac_type& actual_factory()
        {
            return m_actual_factory;
        }

        /**
          * Generates a block instance
          * @param blocktype the type of block
          * @param name the name of the block instance
          * @param active indicates whether the block instance is active or passive
          */
        static std::shared_ptr<Block> 
        instantiate(const std::string &blocktype, std::string name, bool active)
        {
            auto & tf = instance().m_actual_factory;
            return tf.shared(blocktype, std::move(name), std::move(active));
        }


    };

/**
  * Registration class. It wraps a more::factory_register object
  */ 

template<typename blocktype>
class BlockFactoryRegistration : public more::factory_register<bm::Block, blocktype>
{
public:
    /*
     * the object constructor.
     * It runs before the main starts and registers the blocktype block class  to the factory
     * @param name the string which will have to be provided to the factory in order to generate an block of class blocktype
    */ 
    BlockFactoryRegistration(std::string name):
    more::factory_register<bm::Block, blocktype>(bm::BlockFactory::instance().actual_factory(), std::move(name))
    {}
};


}//namespace bm
/**
  * helper macro.
  * It instantiates a BlockFactoryRegistration object for the block class. 
  * This has to be put in the cpp file of the block class after the class definition.
  * @param the block class that needs to be registered
  * @param the string that will be passed to the factory in order for an instance of this class to be generated
 */ 
#define REGISTER_BLOCK(blocktype,name)		\
namespace\
{\
    BlockFactoryRegistration<blocktype> __registration__(name);\
} 


#endif /* _BLOCKFACTORY_HPP_ */
