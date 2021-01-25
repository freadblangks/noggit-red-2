// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainOrientVerticesNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TerrainOrientVerticesNode::TerrainOrientVerticesNode()
: ContextLogicNodeBase()
{
  setName("Terrain :: OrientVertices");
  setCaption("Terrain :: OrientVertices");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Angle<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Orientation<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainOrientVerticesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 const& pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();

  world->orientVertices({pos.x, pos.y, pos.z}, defaultPortData<DecimalData>(PortType::In, 2)->value(),
                        defaultPortData<DecimalData>(PortType::In, 3)->value());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
