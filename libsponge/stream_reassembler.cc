#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), _unassembled_buffer(capacity, ' '), _occupied(capacity, false), _seperator(0), _first_unassembled_index(0), _unassembled_bytes(0), _is_eof_data_received(false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    const size_t stored_bytes = store_data(data, index);        // store the substring(data) as much as possible
    const bool is_all_stored = (stored_bytes == data.size());   // check that there are some of bytes thrown away
    _is_eof_data_received |= eof && is_all_stored;              // update eof data flag

    // reassemble 된 data를 _output으로 옮겨줘야 함.. 옮기고 나면 _unassemled_bytes를 빼준다.

    if(_is_eof_data_received && (_unassembled_bytes == 0)) {    // if all data are written into the output stream in right order,
        _output.end_input();                                    // call end_input() of _output
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return (_unassembled_bytes==0) && _output.buffer_empty(); }


/* private member functions */

size_t StreamReassembler::store_data(const string &data, const size_t index) {
    // compute how many bytes can be stored with finite capacity
    size_t bytes_reassembled_but_not_read = _output.buffer_size();
    size_t bytes_can_be_stored = std::min(data.size(), _capacity - bytes_reassembled_but_not_read - index + _first_unassembled_index);
    
    // decide beginning index
    size_t begin = (index < _first_unassembled_index) ? (_first_unassembled_index - index) : 0;

    // store data into buffer
    for(size_t i = begin ; i < bytes_can_be_stored ; i++) {
        size_t buf_i = (i + index + _seperator - _first_unassembled_index) % _capacity;
        if (!_occupied[buf_i]) {
            _unassembled_bytes++;
        }
        _unassembled_buffer[buf_i] = data[i];
        _occupied[buf_i] = true;
    }

    return bytes_can_be_stored;
}

