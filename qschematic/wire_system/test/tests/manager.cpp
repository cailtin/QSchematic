#include "../3rdparty/doctest.h"
#include "../connector.hpp"
#include "../../manager.hpp"
#include "../../net.hpp"
#include "../../wire.hpp"

TEST_SUITE("Manager")
{
    TEST_CASE ("add_wire(): Wires can be added to the manager")
    {
        wire_system::manager manager;

        auto wire1 = std::make_shared<wire_system::wire>();
        wire1->append_point({0, 0});
        wire1->append_point({10, 0});

        REQUIRE_EQ(wire1->points_count(), 2);

        manager.add_wire(wire1);

        REQUIRE_EQ(std::size(manager.wires()), 1);
        REQUIRE(wire1->net().get());

        auto wire2 = std::make_shared<wire_system::wire>();
        wire2->append_point({10, 10});
        wire2->append_point({10, 20});
        wire2->append_point({20, 20});

        REQUIRE_EQ(wire2->points_count(), 3);

        manager.add_wire(wire2);

        REQUIRE_EQ(std::size(manager.wires()), 2);
        REQUIRE(wire2->net().get());
    }

    TEST_CASE ("generate_junctions(): Junctions can be generated")
    {
        wire_system::manager manager;

        // Create the first wire
        auto wire1 = std::make_shared<wire_system::wire>();
        wire1->append_point({0, 10});
        wire1->append_point({10, 10});
        manager.add_wire(wire1);

        // Create a second wire that lays on the first one
        auto wire2 = std::make_shared<wire_system::wire>();
        wire2->append_point({5, 0});
        wire2->append_point({5, 10});
        manager.add_wire(wire2);

        // Generate the junctions
        manager.generate_junctions();

        // Make sure the wires are connected
        REQUIRE_EQ(std::size(manager.wires_connected_to(wire1)), 2);
        REQUIRE(wire1->net().get());
        REQUIRE_EQ(wire1->net().get(), wire2->net().get());
    }

    TEST_CASE ("connect_wire(): Wire can be connected manually")
    {
        wire_system::manager manager;

        // Create the first wire
        auto wire1 = std::make_shared<wire_system::wire>();
        wire1->append_point({0, 10});
        wire1->append_point({10, 10});
        manager.add_wire(wire1);

        // Create a second wire that lays on the first one
        auto wire2 = std::make_shared<wire_system::wire>();
        wire2->append_point({5, 0});
        wire2->append_point({5, 10});
        manager.add_wire(wire2);

        // Make sure the wires are not connected
        REQUIRE_NE(wire1->net().get(), wire2->net().get());

        // Connect the wires
        manager.connect_wire(wire1.get(), wire2.get(), 1);

        // Make sure the wires are connected
        REQUIRE_EQ(std::size(manager.wires_connected_to(wire1)), 2);
        REQUIRE(wire1->net().get());
        REQUIRE_EQ(wire1->net().get(), wire2->net().get());
        REQUIRE(wire2->points().last().is_junction());
    }

    TEST_CASE ("disconnect_wire(): Wire can be disconnected")
    {
        wire_system::manager manager;

        // Create the first wire
        auto wire1 = std::make_shared<wire_system::wire>();
        wire1->append_point({0, 10});
        wire1->append_point({10, 10});
        manager.add_wire(wire1);

        // Create a second wire that lays on the first one
        auto wire2 = std::make_shared<wire_system::wire>();
        wire2->append_point({5, 0});
        wire2->append_point({5, 10});
        manager.add_wire(wire2);

        // Connect the wires
        manager.connect_wire(wire1.get(), wire2.get(), 1);

        // Make sure the wires are connected
        REQUIRE(wire1->connected_wires().contains(wire2.get()));
        REQUIRE(wire2->points().last().is_junction());

        // Disconnect the wire
        manager.disconnect_wire(wire1, wire2.get());

        // Make sure the wires are not connected
        REQUIRE_FALSE(wire1->connected_wires().contains(wire2.get()));
        REQUIRE_NE(wire1->net().get(), wire2->net().get());
    }

    TEST_CASE ("attach_wire_to_connector(): Attaching a wire to a connector")
    {
        wire_system::manager manager;

        // Create the first wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point({0, 10});
        wire->append_point({10, 10});
        manager.add_wire(wire);

        // Create and attach the connector
        connector conn;

        SUBCASE("The wires is on the connector") {
            // Try to attach the wire to the connector
            conn.pos = QPointF(10, 10);
            manager.attach_wire_to_connector(wire.get(), &conn);

            // Make sure the wire has been attached
            REQUIRE_EQ(manager.attached_wire(&conn), wire.get());
            REQUIRE_EQ(manager.point_is_attached(wire.get(), 0), false);
            REQUIRE_EQ(manager.point_is_attached(wire.get(), 1), true);
        }

            SUBCASE("The wires is not on the connector") {
            // Try to attach the wire to the connector
            conn.pos = QPointF(100, -50);
            manager.attach_wire_to_connector(wire.get(), &conn);

            // Make sure the wire has not been attached
            REQUIRE_EQ(manager.attached_wire(&conn), nullptr);
            REQUIRE_EQ(manager.point_is_attached(wire.get(), 0), false);
            REQUIRE_EQ(manager.point_is_attached(wire.get(), 1), false);
        }
    }

    TEST_CASE ("connector_moved(): Moving a connector with a wire attached")
    {
        wire_system::manager manager;

        // Create the first wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point({0, 10});
        wire->append_point({10, 10});
        manager.add_wire(wire);

        // Create and attach the connector
        connector conn;
        conn.pos = QPointF(10, 10);
        manager.attach_wire_to_connector(wire.get(), &conn);

        // Prepare the settings
        Settings settings;
        settings.gridSize = 1;

        SUBCASE("Straight angles are maintained") {
            // Enable the straight angles
            settings.preserveStraightAngles = true;
            manager.set_settings(settings);

            // Move the connector
            conn.pos = QPointF(10, 20);
            manager.connector_moved(&conn);

            // Make sure every thing is as expected
            REQUIRE_EQ(wire->points_count(), 4);
            REQUIRE_EQ(wire->points().at(0).toPointF(), QPointF(0, 10));
            REQUIRE_EQ(wire->points().at(1).toPointF(), QPointF(5, 10));
            REQUIRE_EQ(wire->points().at(2).toPointF(), QPointF(5, 20));
            REQUIRE_EQ(wire->points().at(3).toPointF(), QPointF(10, 20));
        }

        SUBCASE("Straight angles are not maintained") {
            // Disable the straight angles
            settings.preserveStraightAngles = false;
            manager.set_settings(settings);

            // Move the connector
            conn.pos = QPointF(10, 20);
            manager.connector_moved(&conn);

            // Make sure everything is as expected
            REQUIRE_EQ(wire->points_count(), 2);
            REQUIRE_EQ(wire->points().at(0).toPointF(), QPointF(0, 10));
            REQUIRE_EQ(wire->points().at(1).toPointF(), QPointF(10, 20));
        }
    }

    TEST_CASE("Connections are updated when a points is inserted or removed")
    {
        wire_system::manager manager;

        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(0, 20));
        wire->append_point(QPointF(80, 20));
        manager.add_wire(wire);

        // Create a connector
        connector conn1;
        conn1.pos = QPointF(0, 20);

        // Create a second connector
        connector conn2;
        conn2.pos = QPointF(80, 20);

        // Attach the wire to the connectors
        manager.attach_wire_to_connector(wire.get(), &conn1);
        manager.attach_wire_to_connector(wire.get(), &conn2);

        // Make sure the correct points are attached to the connectors
        REQUIRE_EQ(manager.attached_point(&conn1), 0);
        REQUIRE_EQ(manager.attached_point(&conn2), 1);

        // Insert a point between the two points
        wire->insert_point(1, QPointF(40, 40));

        // Make sure the correct points are attached to the connectors
        REQUIRE_EQ(manager.attached_point(&conn1), 0);
        REQUIRE_EQ(manager.attached_point(&conn2), 2);

        // Prepend a point
        wire->prepend_point(QPointF(0, 20));

        // Make sure the correct points are attached to the connectors
        REQUIRE_EQ(manager.attached_point(&conn1), 0);
        REQUIRE_EQ(manager.attached_point(&conn2), 3);

        // Append a point
        wire->append_point(QPointF(80, 20));

        // Make sure the correct points are attached to the connectors
        REQUIRE_EQ(manager.attached_point(&conn1), 0);
        REQUIRE_EQ(manager.attached_point(&conn2), 4);

        // Simplify the wire
        wire->simplify();

        // Make sure the correct points are attached to the connectors
        REQUIRE_EQ(manager.attached_point(&conn1), 0);
        REQUIRE_EQ(manager.attached_point(&conn2), 2);

        // Remove the middle point
        wire->remove_point(1);

        // Make sure the correct points are attached to the connectors
        REQUIRE_EQ(manager.attached_point(&conn1), 0);
        REQUIRE_EQ(manager.attached_point(&conn2), 1);
    }

    TEST_CASE("global_nets()") {
        SUBCASE("no shared net names") {
            wire_system::manager m;

            // Create & add nets
            {
                auto wn1 = std::make_shared<wire_system::net>();
                wn1->set_name(std::string{"A"});

                auto wn2 = std::make_shared<wire_system::net>();
                wn2->set_name(std::string{"B"});

                auto wn3 = std::make_shared<wire_system::net>();
                wn3->set_name(std::string{"C"});

                m.add_net(wn1);
                m.add_net(wn2);
                m.add_net(wn3);

                // Sanity check
                CHECK_EQ(std::size(m.nets()), 3);
            }

            // Generate global nets and check stuff
            {
                const auto gn = m.global_nets();
                REQUIRE_EQ(std::size(gn), 3);

                CHECK_EQ(gn[0].name, "A");
                CHECK_EQ(std::size(gn[0].nets), 1);

                CHECK_EQ(gn[1].name, "B");
                CHECK_EQ(std::size(gn[1].nets), 1);

                CHECK_EQ(gn[2].name, "C");
                CHECK_EQ(std::size(gn[2].nets), 1);
            }
        }

        SUBCASE("some shared net names") {
            wire_system::manager m;

            // Create & add nets
            {
                auto wn1 = std::make_shared<wire_system::net>();
                wn1->set_name(std::string{"A"});

                auto wn2 = std::make_shared<wire_system::net>();
                wn2->set_name(std::string{"B"});

                auto wn3 = std::make_shared<wire_system::net>();
                wn3->set_name(std::string{"C"});

                auto wn4 = std::make_shared<wire_system::net>();
                wn4->set_name(std::string{"A"});

                m.add_net(wn1);
                m.add_net(wn2);
                m.add_net(wn3);
                m.add_net(wn4);

                // Sanity check
                CHECK_EQ(std::size(m.nets()), 4);
            }

            // Generate global nets and check stuff
            {
                const auto gn = m.global_nets();
                REQUIRE_EQ(std::size(gn), 3);

                CHECK_EQ(gn[0].name, "A");
                CHECK_EQ(std::size(gn[0].nets), 2);

                CHECK_EQ(gn[1].name, "B");
                CHECK_EQ(std::size(gn[1].nets), 1);

                CHECK_EQ(gn[2].name, "C");
                CHECK_EQ(std::size(gn[2].nets), 1);
            }
        }

        SUBCASE("all anonymous net names") {
            wire_system::manager m;

            // Create & add nets
            m.add_net(std::make_shared<wire_system::net>());
            m.add_net(std::make_shared<wire_system::net>());
            m.add_net(std::make_shared<wire_system::net>());
            m.add_net(std::make_shared<wire_system::net>());

            // Generate global nets and check stuff
            {
                const auto gn = m.global_nets();
                REQUIRE_EQ(std::size(gn), 4);

                CHECK_EQ(gn[0].name, "N001");
                CHECK_EQ(std::size(gn[0].nets), 1);

                CHECK_EQ(gn[1].name, "N002");
                CHECK_EQ(std::size(gn[0].nets), 1);

                CHECK_EQ(gn[2].name, "N003");
                CHECK_EQ(std::size(gn[0].nets), 1);

                CHECK_EQ(gn[3].name, "N004");
                CHECK_EQ(std::size(gn[0].nets), 1);
            }
        }

        SUBCASE("some anonymous net names with shared net names") {
            wire_system::manager m;

            // Create & add nets
            {
                auto wn1 = std::make_shared<wire_system::net>();
                wn1->set_name(std::string{"A"});

                auto wn2 = std::make_shared<wire_system::net>();
                wn2->set_name(std::string{""});

                auto wn3 = std::make_shared<wire_system::net>();
                wn3->set_name(std::string{""});

                auto wn4 = std::make_shared<wire_system::net>();
                wn4->set_name(std::string{"A"});

                auto wn5 = std::make_shared<wire_system::net>();
                wn5->set_name(std::string{"B"});

                auto wn6 = std::make_shared<wire_system::net>();
                wn6->set_name(std::string{""});

                m.add_net(wn1);
                m.add_net(wn2);
                m.add_net(wn3);
                m.add_net(wn4);
                m.add_net(wn5);
                m.add_net(wn6);

                // Sanity check
                CHECK_EQ(std::size(m.nets()), 6);
            }

            // Generate global nets and check stuff
            {
                const auto gn = m.global_nets();
                REQUIRE_EQ(std::size(gn), 5);

                CHECK_EQ(gn[0].name, "A");
                CHECK_EQ(std::size(gn[0].nets), 2);

                CHECK_EQ(gn[1].name, "N001");
                CHECK_EQ(std::size(gn[1].nets), 1);

                CHECK_EQ(gn[2].name, "N002");
                CHECK_EQ(std::size(gn[2].nets), 1);

                CHECK_EQ(gn[3].name, "B");
                CHECK_EQ(std::size(gn[3].nets), 1);

                CHECK_EQ(gn[4].name, "N003");
                CHECK_EQ(std::size(gn[4].nets), 1);
            }
        }
    }
}
