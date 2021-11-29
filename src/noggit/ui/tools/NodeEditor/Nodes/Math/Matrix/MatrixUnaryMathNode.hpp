// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_MATRIXUNARYMATHNODE_HPP
#define NOGGIT_MATRIXUNARYMATHNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp"
#include <QComboBox>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace ui::tools::NodeEditor::Nodes
    {
        class MatrixUnaryMathNode : public BaseNode
        {
        Q_OBJECT

        public:
          MatrixUnaryMathNode();
          void compute() override;
          NodeValidationState validate() override;
          QJsonObject save() const override;
          void restore(QJsonObject const& json_obj) override;

        private:
          QComboBox* _operation;
        };

    }

}

#endif //NOGGIT_MATRIXUNARYMATHNODE_HPP