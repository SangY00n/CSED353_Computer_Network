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

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {     
    _time_since_last_segment_received=0;
    const TCPHeader &header = seg.header();

    // if the connection is not active yet, activate the connection only if the SYN flag is received
    if(!_active) {
        if(header.syn) {
            _active = true;
            _receiver.segment_received(seg);
            if(_syn_sent) {
                _syn_acked=true;
            } else {
                connect();
            }
        }
        return;
    }

    // if the segment has the RST flag
    if(header.rst) {
        _active = false; // connection is dead, active() should return false
        
        // set the error flag on the inbound and outbound ByteStreams
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();

        return;
    }

    _receiver.segment_received(seg); // call the receiver's segment_received() to pass the segment

    // if the segment has the ACK flag
    if(header.ack) {
        // call the sender's ack_received() to pass the remote peer's receiver's ackno and window_size
        _sender.ack_received(header.ackno, header.win); 
    }
    
    // if local receiver has ackno
    if(_receiver.ackno().has_value()) {
        _sender.fill_window();
    }

    if(_fin_sent) {
        if(header.ackno==_sender.next_seqno()) { // if local's FIN segment was acked
            _fin_acked=true; // set _fin_acked
            // if _linger_after_streams_finish is false, there's no need to lingering. so connection is over...
            _active = _active && _linger_after_streams_finish; // set _active false if linger_after_streams_finish is false
        }
    } else {
        // if remote peer was the first one to end its stream. -> passive close
        _linger_after_streams_finish = _linger_after_streams_finish && !_receiver.stream_out().input_ended();
    }

    // send ack only to remote peer
    if(seg.length_in_sequence_space()>0 && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }

    // respond to a "keep-alive" segment even though they do not occupy any sequence numbers
    if(_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) && (seg.header().seqno == _receiver.ackno().value() - 1)) {
        _sender.send_empty_segment();
    }

    flush_sender(); // send all segments
 }

bool TCPConnection::active() const { return _active; }

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
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    _time_since_last_segment_received += ms_since_last_tick; // update time_since_last_segment_received
    _sender.tick(ms_since_last_tick); // call sender's tick method
    ////////////////////////////////////////////////////////
 }

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

    _active=true;
    _syn_sent=true;
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
        if(_receiver.ackno().has_value()) { // if there is an ackno,
            front_seg.header().ackno = _receiver.ackno().value();
            front_seg.header().ack = true; // set ack flag
        }
        front_seg.header().win = _receiver.window_size() < numeric_limits<uint16_t>::max() ? _receiver.window_size() : numeric_limits<uint16_t>::max();

        // push(send) the segment into outboud queue
        _segments_out.push(front_seg);

        if(front_seg.header().fin) { // if FIN flag is set
            _fin_sent=true; // set _fin_sent
        }
    }
}