#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return {}; }

void TCPSender::fill_window() {}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { DUMMY_CODE(ackno, window_size); }

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

unsigned int TCPSender::consecutive_retransmissions() const { return {}; }

void TCPSender::send_empty_segment() {}




RetransmissionTimer::RetransmissionTimer(const uint16_t retx_timeout)
    : _init_rto(retx_timeout)
    , _rto(retx_timeout)
    , _t(0)
    , _is_running(true) {}

bool RetransmissionTimer::tick(const size_t ms_since_last_tick) {
    bool is_expired = false;
    
    // check if timer expired by comparing running time with rto value
    is_expired = _t + ms_since_last_tick >= _rto;

    // update _t
    _t = is_expired ? 0 : _t + ms_since_last_tick;

    return is_expired;  
}
bool RetransmissionTimer::is_running() {
    return _is_running;
}
void RetransmissionTimer::start() {
    _t = 0;
    _is_running = true;
}
void RetransmissionTimer::stop() {
    _t = 0;
    _is_running = false;
}