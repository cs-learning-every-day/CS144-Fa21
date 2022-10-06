#include "tcp_receiver.hh"

#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    auto header = seg.header();
    auto payload = seg.payload();
    if (header.syn) {
        _syn_flag = true;
        _isn_no = header.seqno;
    }
    if (!_syn_flag) {
        // std::cerr << "??" << std::endl;
        return;
    }
    if (header.fin) {
        _fin_flag = true;
    }
    // std::cerr << "_isn_no: " << _isn_no << std::endl;
    uint64_t stream_idx = unwrap(header.seqno, _isn_no, 0) - (header.syn ? 0 : 1);
    // std::cerr << "payload: " << payload.copy() << " stream_idx: " << stream_idx << std::endl;
    _reassembler.push_substring(payload.copy(), stream_idx, header.fin);
    // std::cerr << "buffer writens: " << stream_out().bytes_written() << std::endl;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_flag) {
        return std::nullopt;
    }
    return _isn_no + stream_out().bytes_written() + 1 +
           ((_fin_flag && _reassembler.empty()) ? 1 : 0);
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
