/* WARNING this block is deprecated; see base/blocks/IpfixExport.cpp */

#include <Exporter.h>
#include <Block.hpp>

namespace bm
{

class IpfixExportBlock: public Block {
  
public:
  
  static const uint16_t TUPLE_STATISTIC_TID = 0x7757;
  static const uint16_t TUPLE_TID = 0x7758;
    
  IpfixExportBlock(const std::string &name, bool active);
                 
  virtual ~IpfixExportBlock();
  
  /* Idiomatically uncopyable */
  IpfixExportBlock(const IpfixExportBlock &) = delete;
  IpfixExportBlock& operator=(const IpfixExportBlock &) = delete;
  
  virtual void _configure(const xml_node& n);
  virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int index); 


private:

  void add_struct_templates();
  void add_export_templates();

  IPFIX::InfoModel*                   model_;
  IPFIX::Exporter*                    exporter_;
  std::map<int, const IPFIX::IETemplate*>    stmap_;
  std::map<int, uint16_t>             tidmap_;
  int                                 ingate_;
};

}
