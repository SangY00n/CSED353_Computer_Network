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
    , _stream(capacity)
    , _retx_timer(retx_timeout)
    , _previous_ackno(0)
    , _window_size(1)
    , _bytes_in_flight(0)
    , _consecutive_retransmissions(0)
    , _is_fin_sent(false) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    // If the receiver has announced a window size of 0, fill_window() should act like the window size is 1
    uint16_t window_size = _window_size == 0 ? 1 : _window_size;
    uint64_t available_window_size = window_size - (next_seqno_absolute() - _previous_ackno);

    TCPSegment syn_seg;
    // CLOSED state : waiting for stream to begin (no SYN sent)
    if (next_seqno_absolute() == 0) {
        syn_seg.header().syn = 1;
        syn_seg.header().seqno = wrap(_next_seqno, _isn);
        _segments_out.push(syn_seg);
        _outstanding_segments.push(syn_seg);
        _retx_timer.start();
        _next_seqno += syn_seg.length_in_sequence_space();
        _bytes_in_flight += syn_seg.length_in_sequence_space();
        return;
    }
    // SYN_SENT state : stream started but nothing acknowledged
    if (next_seqno_absolute() > 0 && next_seqno_absolute() == bytes_in_flight()) {
        return;
    }

    while (available_window_size > 0) {
        TCPSegment temp_seg;
        if (_stream.eof()) {
            if (!_is_fin_sent) {
                // SYN_ACKED2 state (FIN flag has't been sent yet)
                temp_seg.header().fin = 1;
                _is_fin_sent = true;
                temp_seg.header().seqno = wrap(_next_seqno, _isn);
                _segments_out.push(temp_seg);
                _outstanding_segments.push(temp_seg);
                _retx_timer.start();
                _next_seqno += temp_seg.length_in_sequence_space();
                _bytes_in_flight += temp_seg.length_in_sequence_space();
            }
            // FIN_SENT state : stream finished (FIN sent) but not fully acknowledged
            break;
        }

        // SYN_ACKED1 state
        // make segment no bigger than MAX_PAYLOAD_SIZE (1452 bytes)
        size_t bytes_to_send =
            available_window_size < TCPConfig::MAX_PAYLOAD_SIZE ? available_window_size : TCPConfig::MAX_PAYLOAD_SIZE;
        temp_seg.payload() = Buffer(_stream.read(bytes_to_send));
        if (window_size - temp_seg.length_in_sequence_space() > 0 && _stream.eof()) {
            // there is remaining window space and stream has reached EOF
            temp_seg.header().fin = 1;
            _is_fin_sent = true;
        }
        if (temp_seg.length_in_sequence_space() != 0) {
            temp_seg.header().seqno = wrap(_next_seqno, _isn);
            _segments_out.push(temp_seg);
            _outstanding_segments.push(temp_seg);
            _retx_timer.start();
            _next_seqno += temp_seg.length_in_sequence_space();
            _bytes_in_flight += temp_seg.length_in_sequence_space();
        } else {
            // FIN_SENT state
            break;
        }

        available_window_size = window_size - (next_seqno_absolute() - _previous_ackno);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t unwrapped_ackno = unwrap(ackno, _isn, _previous_ackno);
    _window_size = window_size;

    // Impossible ackno (beyond next seqno) is ignored
    if (unwrapped_ackno > _next_seqno)
        return;

    // 만약 이전에 도착한 가장 큰 ackno(_previous_ackno) 보다 작거나 같은 경우.. -> 아무것도 안해도 됨
    if (unwrapped_ackno <= _previous_ackno)
        return;

    // When the receiver gives the sender an ackno that acknowledges the successful receipt of new data
    // ( the ackno bigger than any previous ackno )
    // 이전에 도착한 가장 큰 ackno(_previous_ackno) 보다 크다면, _previous_ackno 업데이트한 뒤
    _previous_ackno = unwrapped_ackno;
    // Set the RTO back to its "initial value"
    _retx_timer.set_rto_init();
    // Restart the retransmission timer
    _retx_timer.stop();
    _retx_timer.start();
    // If the sender has any outstading data, restart the retransmission timer so that it will expire after RTO
    // milliseconds collection of outstanding segments 를 확인해야 함
    while (!_outstanding_segments.empty()) {
        const TCPSegment &temp_seg = _outstanding_segments.front();
        uint64_t seg_unwrapped_seqno = unwrap(temp_seg.header().seqno, _isn, _previous_ackno);
        uint64_t temp_seg_size = temp_seg.length_in_sequence_space();
        if (seg_unwrapped_seqno + temp_seg.length_in_sequence_space() <= unwrapped_ackno) {
            // temp_seg is fully acknowledged outstanding segment
            _outstanding_segments.pop();
            _bytes_in_flight -= temp_seg_size;
        } else {
            // there's no more fully acknowledged outstanding segment
            break;
        }
    }
    // Reset the count of "consecutive retransmissions" back to zero
    _consecutive_retransmissions = 0;

    // fill the window again if new space has opened up
    fill_window();

    // if there exists outstanding segment, restart the retransmission timer
    if (_outstanding_segments.size() > 0) {
        _retx_timer.stop();
        _retx_timer.start();
    }
    // When all outstanding data has been acknowledged, stop the retransmission timer.
    else {
        _retx_timer.stop();
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    bool is_expired = _retx_timer.tick(ms_since_last_tick);

    if (is_expired) {  // If the timer has expired
        // Retransmit the earliest segment that hasn't been fully acknowledged by the TCP receiver.
        if (!_outstanding_segments.empty()) {
            _segments_out.push(_outstanding_segments.front());
        }

        // If the window size is nonzero:
        if (_window_size != 0) {
            // i. increase # consecutive retransmissions beacuse I just retransmitted something
            _consecutive_retransmissions++;
            // ii. Double the value of RTO - "exponential backoff"
            _retx_timer.double_rto();
        }

        // Reset the timer and start it
        _retx_timer.stop();
        _retx_timer.start();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    // A segment that occupies no sequence numbers (no payload, SYN or FIN) doesn't need to be remembered or
    // retransmitted. Thus, there's no need to store empty segments in outstanding queue
    TCPSegment empty_seg;
    empty_seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(empty_seg);
}

RetransmissionTimer::RetransmissionTimer(const uint16_t retx_timeout)
    : _init_rto(retx_timeout), _rto(retx_timeout), _t(0), _is_running(true) {}

bool RetransmissionTimer::tick(const size_t ms_since_last_tick) {
    bool is_expired = false;
    if (!_is_running)
        return false;

    // check if timer expired by comparing running time with rto value
    is_expired = _t + ms_since_last_tick >= _rto;

    // update _t
    _t = is_expired ? 0 : _t + ms_since_last_tick;

    return is_expired;
}
void RetransmissionTimer::start() {
    if (!_is_running)
        _t = 0;
    _is_running = true;
}
void RetransmissionTimer::stop() {
    _t = 0;
    _is_running = false;
}
void RetransmissionTimer::set_rto_init() { _rto = _init_rto; }
void RetransmissionTimer::double_rto() { _rto *= 2; }