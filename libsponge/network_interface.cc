#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    EthernetFrame frame_to_send;
    EthernetHeader frame_header;

    // if IP addr - ethernet addr pair is cached (the est Ethernet addr is already known)
    if (_ethernet_addr_cache.count(next_hop_ip) != 0) {
        frame_header.src = _ethernet_address;
        frame_header.dst = _ethernet_addr_cache[next_hop_ip].first;
        frame_header.type = EthernetHeader::TYPE_IPv4;
        frame_to_send.header() = frame_header;
        frame_to_send.payload() = dgram.serialize();

        _frames_out.push(frame_to_send);
    }
    // if not cached (the dst Ethernet addr is unknown)
    else {
        // broadcast an ARP for the next hop's Ethernet addr
        if (_arp_timer.count(next_hop_ip) == 0 || _arp_timer[next_hop_ip] >= ARP_REQ_TIME) {
            _arp_timer[next_hop_ip] = 0; // initialize the timer for the next_hop_ip

            ARPMessage arp_req;
            arp_req.opcode = ARPMessage::OPCODE_REQUEST;
            arp_req.sender_ethernet_address = _ethernet_address;
            arp_req.sender_ip_address = _ip_address.ipv4_numeric();
            arp_req.target_ethernet_address = EthernetAddress{};
            arp_req.target_ip_address = next_hop_ip;

            frame_header.src = _ethernet_address;
            frame_header.dst = ETHERNET_BROADCAST;
            frame_header.type = EthernetHeader::TYPE_ARP;
            frame_to_send.header() = frame_header;
            frame_to_send.payload() = arp_req.serialize();

            _frames_out.push(frame_to_send);
        }
        // queue the IP datagram -> it can be sent after the ARP reply is received
        _queueing_IP_datagrams.push_back(make_pair(dgram, next_hop));
    }   
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    DUMMY_CODE(frame);
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }
