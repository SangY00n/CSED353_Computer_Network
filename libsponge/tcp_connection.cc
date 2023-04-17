#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return {}; }

void TCPConnection::segment_received(const TCPSegment &seg) { DUMMY_CODE(seg); }

bool TCPConnection::active() const { return {}; }

size_t TCPConnection::write(const string &data) {
    // write the data into the outbound stream
    size_t accepted_bytes = _sender.stream_in().write(data);
    // create segments to send
    _sender.fill_window();
    // send all segments
    flush_sender();

    return accepted_bytes;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

void TCPConnection::end_input_stream() {
    // give the end_input signal to the outbound stream
    _sender.stream_in().end_input();
    // create segments if possible
    _sender.fill_window();
    // send all segments
    flush_sender();
}

void TCPConnection::connect() {
    _sender.fill_window(); // create a SYN segment
    TCPSegment syn_seg = _sender.segments_out().front();
    _sender.segments_out().pop();
    _segments_out.push(syn_seg);
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::flush_sender() {
    while (!_sender.segments_out().empty()) {
        TCPSegment front_seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        // before sending the segment, ask the TCPReceiver for ackno and window size
        if(_receiver.ackno() != nullopt) { // if there is an ackno,
            front_seg.header().ackno = _receiver.ackno().value();
            front_seg.header().ack = true; // set ack flag
        }
        front_seg.header().win = _receiver.window_size();

        // push(send) the segment into outboud queue
        _segments_out.push(front_seg);
    }
}