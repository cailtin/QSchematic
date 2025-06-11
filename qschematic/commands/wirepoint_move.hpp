#pragma once

#include "base.hpp"
#include "../items/wire.hpp"

#include <QVector>
#include <QVector2D>

#include <memory>

class QVector2D;

namespace QSchematic::Commands
{

    class WirepointMove :
        public Base
    {
    public:
        WirepointMove(
            Scene* scene,
            const std::shared_ptr<Items::Wire>& wire,
            int index,
            const QPointF& pos,
            QUndoCommand* parent = nullptr
        );

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        std::shared_ptr<Items::Wire> _wire;
        QVector<QPointF> _oldPos;
        QVector<QPointF> _newPos;
        std::shared_ptr<net> _oldNet;
        std::shared_ptr<net> _newNet;
        Scene* _scene = nullptr;
    };

}
