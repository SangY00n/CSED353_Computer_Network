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
            _arp_timer[next_hop_ip] = 0;  // initialize the timer for the next_hop_ip

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
        _queueing_IP_datagrams.push_back(make_pair(dgram, next_hop_ip));
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    const EthernetHeader &frame_header = frame.header();

    // should ignore any frames not destined for the network interface
    // (dst is either the broadcase addr or the interface's own Ethernet addr)
    if (!(frame_header.dst == ETHERNET_BROADCAST || frame_header.dst == _ethernet_address)) {
        return {};
    }

    // if frame is IPv4,
    if (frame_header.type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        ParseResult parsing_result = dgram.parse(frame.payload());  // parse the payload as an InternetDatagram
        if (parsing_result == ParseResult::NoError) {               // if successful
            return dgram;
        } else {  // if not successful
            return {};
        }
    }
    // if frame is ARP,
    else {
        ARPMessage msg;
        ParseResult parsing_result = msg.parse(frame.payload());  // parse the payload as an ARPMessage
        if (parsing_result == ParseResult::NoError) {             // if successful
            // cache the mapping btw sender's IP addr and Ethernet addr
            _ethernet_addr_cache[msg.sender_ip_address] = make_pair(msg.sender_ethernet_address, 0);

            // if sender's IP addr match with our sent ARP request, update _arp_timer
            if (_arp_timer.count(msg.sender_ip_address) != 0) {
                _arp_timer.erase(msg.sender_ip_address);
            }

            // if the ARP message is the ARP reply corresponding to the ARP request broadcasted before,
            // send queued IP datagram
            std::list<std::pair<InternetDatagram, uint32_t>>::iterator it = _queueing_IP_datagrams.begin();
            while (it != _queueing_IP_datagrams.end()) {
                if ((*it).second == msg.sender_ip_address) {
                    send_datagram((*it).first, Address::from_ipv4_numeric((*it).second));

                    _queueing_IP_datagrams.erase(it++);
                } else {
                    it++;
                }
            }

            // if the ARP message is an ARP request asking for our IP addr, send an appropriate ARP reply
            if (msg.opcode == ARPMessage::OPCODE_REQUEST && msg.target_ip_address == _ip_address.ipv4_numeric()) {
                ARPMessage arp_rep;
                arp_rep.opcode = ARPMessage::OPCODE_REPLY;
                arp_rep.sender_ethernet_address = _ethernet_address;
                arp_rep.sender_ip_address = _ip_address.ipv4_numeric();
                arp_rep.target_ethernet_address = msg.sender_ethernet_address;
                arp_rep.target_ip_address = msg.sender_ip_address;

                EthernetFrame frame_to_send;
                EthernetHeader header_to_send;
                header_to_send.src = _ethernet_address;
                header_to_send.dst = msg.sender_ethernet_address;
                header_to_send.type = EthernetHeader::TYPE_ARP;
                frame_to_send.header() = header_to_send;
                frame_to_send.payload() = arp_rep.serialize();

                _frames_out.push(frame_to_send);
            }
        } else {  // if not successful
            return {};
        }
    }

    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    std::list<uint32_t> keys_to_erase;

    for (auto &i : _ethernet_addr_cache) {
        i.second.second += ms_since_last_tick;
        if (i.second.second >= CACHE_VALID_TIME) {
            keys_to_erase.push_back(i.first);
        }
    }

    for (auto &k : keys_to_erase) {
        std::unordered_map<uint32_t, std::pair<EthernetAddress, size_t>>::iterator it = _ethernet_addr_cache.find(k);
        _ethernet_addr_cache.erase(it);
    }

    for (auto &i : _arp_timer) {
        i.second += ms_since_last_tick;
    }
}
