#include "PrintNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"
#include <noggit/Log.h>

using namespace noggit::Red::PresetEditor::Nodes;

PrintNode::PrintNode()
: BaseNode()
{
  setName("PrintNode");
  setCaption("Print()");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addWidget(new QLabel(&_embedded_widget), 0);

  addPort<DecimalData>(PortType::In, "String", true);
  _text = new QLineEdit(&_embedded_widget);
  addWidget(_text, 1);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);

}

void PrintNode::compute()
{
  auto logic = _in_ports[0].in_value.lock();
  auto logic_ptr = static_cast<LogicData*>(logic.get());

  if (!logic_ptr)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);

    return;
  }

  setValidationState(NodeValidationState::Valid);

  if(!logic_ptr->value())
    return;

  auto text = _in_ports[1].in_value.lock();
  auto text_ptr = static_cast<StringData*>(text.get());

  auto msg = text_ptr ? text_ptr->value() : _text->text().toStdString();

  LogDebug << msg << std::endl;

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);
}

QJsonObject PrintNode::save() const
{
  QJsonObject json_obj;
  json_obj["name"] = name();
  json_obj["caption"] = caption();
  json_obj["text"] = _text->text();

  return json_obj;
}

void PrintNode::restore(const QJsonObject& json_obj)
{
  setName(json_obj["name"].toString());
  setCaption(json_obj["caption"].toString());
  _text->setText(json_obj["text"].toString());
}

