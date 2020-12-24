// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CONTEXT_HPP
#define NOGGIT_CONTEXT_HPP

#include <noggit/World.h>
#include "NodeScene.hpp"
#include <external/tsl/robin_map.h>

#include <QObject>
#include <QJsonDocument>

namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
        enum NodeExecutionContext
        {
            MAP_VIEW,
            PRESET_EDITOR,
            ASSET_BROWSER
        };

        class Context : QObject
        {
            Q_OBJECT

        public:
            Context(NodeExecutionContext context_type, QObject* parent = nullptr);

            NodeScene* getScene(QString const& path, QObject* parent = nullptr);
            NodeExecutionContext getContextType() { return _context_type; };
            void makeCurrent();

        private:
            World* _world;
            tsl::robin_map<std::string, QJsonDocument> _scene_cache;
            NodeExecutionContext _context_type;

        };

        extern Context* gCurrentContext;
        extern std::shared_ptr<DataModelRegistry> gDataModelRegistry;
    }
}

#endif //NOGGIT_CONTEXT_HPP