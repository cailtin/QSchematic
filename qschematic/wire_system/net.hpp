#pragma once

#include <QList>

#include <memory>

namespace wire_system
{

    class wire;
    class manager;

    class net :
        public std::enable_shared_from_this<net>
    {
    public:
        net() = default;
        net(const net&) = delete;
        net(net&&) = delete;
        virtual ~net() = default;

        void
        set_name(const std::string& name);

        virtual
        void
        set_name(const QString& name);

        [[nodiscard]]
        QString
        name() const;

        [[nodiscard]]
        QList<std::shared_ptr<wire>>
        wires() const;

        virtual
        bool
        addWire(const std::shared_ptr<wire>& wire);

        virtual
        bool
        removeWire(const std::shared_ptr<wire> wire);

        [[nodiscard]]
        bool
        contains(const std::shared_ptr<wire>& wire) const;

        void
        set_manager(wire_system::manager* manager);

    protected:
        [[nodiscard]]
        class manager*
        manager() const noexcept
        {
            return m_manager;
        }

    private:
        QList<std::weak_ptr<wire>> m_wires;
        class manager* m_manager = nullptr;
        QString m_name;
    };

}
