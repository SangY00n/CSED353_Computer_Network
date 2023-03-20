#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader tcp_seg_header = seg.header();
    bool syn = tcp_seg_header.syn;
    bool fin = tcp_seg_header.fin;
    WrappingInt32 seqno = tcp_seg_header.seqno;
    const string data = seg.payload().copy();

    if((_isn == nullptr) && syn) {
        _isn.reset(new WrappingInt32(seqno.raw_value()));   // initialize ISN(Initial Sequence Number)
        _reassembler.push_substring(data, 0, fin);          // push data into the stream
    } else {
        uint64_t abs_64bit_ackno = _reassembler.stream_out().bytes_written() + 1;
        uint64_t abs_64bit_seqno = unwrap(seqno, *_isn, abs_64bit_ackno);
        if(abs_64bit_seqno!=0) {
            _reassembler.push_substring(data, abs_64bit_seqno - 1, fin);
        }
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const { return {}; }

size_t TCPReceiver::window_size() const { return {}; }
