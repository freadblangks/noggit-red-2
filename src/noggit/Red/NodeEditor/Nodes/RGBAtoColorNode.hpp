// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_RGBATOCOLORNODE_HPP
#define NOGGIT_RGBATOCOLORNODE_HPP

#include "BaseNode.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
        class RGBAtoColorNode : public BaseNode
        {
        Q_OBJECT

        public:
            RGBAtoColorNode();
            void compute() override;
            NodeValidationState validate() override;

        };

    }

}

#endif //NOGGIT_RGBATOCOLORNODE_HPP