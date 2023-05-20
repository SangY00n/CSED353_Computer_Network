#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    // Add a route (forwarding rule) to the sorted routing table in descending order of prefix_length.
    for (auto iter = _routing_table.begin(); iter != _routing_table.end(); iter++) {
        if (prefix_length > (*iter).prefix_length) {
            _routing_table.insert(iter, {route_prefix, prefix_length, next_hop, interface_num});
            return;
        }
    }
    _routing_table.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    // The router decrements the datagram's TTL(time to live).
    // Only if the TTL is larger than 1, the router decrements it.
    if (dgram.header().ttl > 1) {
        dgram.header().ttl--;
    }
    // Otherwise, the router should drop the datagram...
    else {
        return;
    }

    // Since the _routing_table is sorted in descending order of prefix_length,
    // the first match will be the "longest-prefix match".
    for (auto &r : _routing_table) {
        const uint32_t bitmask = (r.prefix_length == 0) ? static_cast<uint32_t>(0)
                                                        : (static_cast<uint32_t>(-1) << (32 - int(r.prefix_length)));
        const uint32_t dst_addr_prefix = ((dgram.header().dst) & bitmask);
        const uint32_t r_prefix = ((r.route_prefix) & bitmask);

        // If the route r matches with dst addr, send datagram.
        if (r_prefix == dst_addr_prefix) {
            // If the next_hop is an empty optional, the next_hop for send_datagram() is the datagram's destination
            // address.
            if (!r.next_hop.has_value()) {
                interface(r.interface_num).send_datagram(dgram, Address::from_ipv4_numeric(dgram.header().dst));
            }
            // Otherwise, the route's next_hop contains the IP address of the next router along the path.
            else {
                interface(r.interface_num).send_datagram(dgram, r.next_hop.value());
            }

            break;
        }
    }

    // If no routes matched, the router drops the datagram...
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
