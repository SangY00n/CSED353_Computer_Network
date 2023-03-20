#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

#define TWOTOTHETHIRTYTWO 0x100000000

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t abs_seqno32 = n & 0xFFFFFFFF;
    return isn + abs_seqno32;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint32_t abs_seqno =  n >= isn ? n - isn : (TWOTOTHETHIRTYTWO - isn.raw_value()) + n.raw_value();  // 0 ~ 2^32 - 1 사이 값만 나옴
    if(abs_seqno >= checkpoint) {
        return abs_seqno;
    }

    uint64_t checkpoint_high_32bit = (checkpoint / TWOTOTHETHIRTYTWO) * TWOTOTHETHIRTYTWO;
    uint32_t checkpoint_low_32bit = checkpoint & 0xFFFFFFFF;
    uint64_t nearby_abs_seqno;

    if(checkpoint_low_32bit > abs_seqno) {
        nearby_abs_seqno = (checkpoint_low_32bit - abs_seqno) >= (TWOTOTHETHIRTYTWO/2) ? checkpoint_high_32bit + abs_seqno + TWOTOTHETHIRTYTWO : checkpoint_high_32bit + abs_seqno;
    } else {
        nearby_abs_seqno = (abs_seqno - checkpoint_low_32bit) >= (TWOTOTHETHIRTYTWO/2) ? checkpoint_high_32bit + abs_seqno - TWOTOTHETHIRTYTWO : checkpoint_high_32bit + abs_seqno;
    }

    return nearby_abs_seqno;
}
