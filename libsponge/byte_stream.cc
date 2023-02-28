#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) { total_capacity = capacity; }

size_t ByteStream::write(const string &data) {
    const size_t cur_capacity = remaining_capacity();
    size_t written_bytes;
    if (cur_capacity <= 0) {
        written_bytes = 0;
    } else if (data.size() > cur_capacity) {
        buffer.append(data, 0, cur_capacity);
        written_bytes = cur_capacity;
    } else {
        buffer.append(data);
        written_bytes = data.size();
    }
    total_written_bytes += written_bytes;
    return written_bytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string copied_str;
    if (buffer_size() > len) {
        copied_str = buffer.substr(0, len);
    } else {
        copied_str = buffer;
    }
    return copied_str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if (buffer.size() > len) {
        total_read_bytes += len;
        buffer = buffer.substr(len);
    } else {
        total_read_bytes += buffer.size();
        buffer = "";
    }

    if (input_ended()) {
        is_eof = total_written_bytes == total_read_bytes;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const string read_str = peek_output(len);
    pop_output(len);

    return read_str;
}

void ByteStream::end_input() {
    is_input_ended = true;
    is_eof = total_written_bytes == total_read_bytes;
}

bool ByteStream::input_ended() const { return is_input_ended; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return is_eof; }

size_t ByteStream::bytes_written() const { return total_written_bytes; }

size_t ByteStream::bytes_read() const { return total_read_bytes; }

size_t ByteStream::remaining_capacity() const { return total_capacity - buffer.size(); }
