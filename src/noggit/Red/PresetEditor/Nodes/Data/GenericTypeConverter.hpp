#ifndef NOGGIT_GENERICTYPECONVERTER_HPP
#define NOGGIT_GENERICTYPECONVERTER_HPP

#include <external/NodeEditor/include/nodes/NodeDataModel>
#include "GenericData.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;

template <typename T_from, typename T_to, typename C>
class GenericTypeConverter
{

public:

    std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data)
    {
      auto data_ptr = static_cast<T_from*>(data.get());

      if (data_ptr)
      {
        _data = std::make_shared<T_to>(C::convert(data_ptr->value()));
      }
      else
      {
        _data.reset();
      }

      return _data;
    };

private:

    std::shared_ptr<NodeData> _data;
};

template<typename T_from, typename T_to>
struct UnderlyingTypeConvertGeneric
{
  static T_to convert(T_from const& value)
  {
    return static_cast<T_to>(value);
  }
};



#define DECLARE_TYPE_CONVERTER(DATA_FROM, DATA_TO, UTYPE_FROM, UTYPE_TO)  \
  using DATA_FROM##To##DATA_TO##TypeConverter =                           \
  GenericTypeConverter<DATA_FROM##Data, DATA_TO##Data,                    \
  UnderlyingTypeConvertGeneric<UTYPE_FROM, UTYPE_TO>>;


DECLARE_TYPE_CONVERTER(Decimal, Integer, double, int)
DECLARE_TYPE_CONVERTER(Decimal, Boolean, double, bool)

DECLARE_TYPE_CONVERTER(Integer, Decimal, int, double)
DECLARE_TYPE_CONVERTER(Integer, Boolean, int, bool)

DECLARE_TYPE_CONVERTER(Boolean, Integer, bool, int)
DECLARE_TYPE_CONVERTER(Boolean, Decimal, bool, double)
//DECLARE_TYPE_CONVERTER(Decimal, String)
//DECLARE_TYPE_CONVERTER(Integer, String)



#endif //NOGGIT_GENERICTYPECONVERTER_HPP