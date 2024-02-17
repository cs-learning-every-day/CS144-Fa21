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
    if (_isn_no == std::nullopt && !header.syn) {
        // std::cerr << "??" << std::endl;
        return;
    }

    if (header.syn) {
        _isn_no = header.seqno;
    }

    // std::cerr << "_isn_no: " << _isn_no << std::endl;
    // 获取reassembler的index，即Seq->AbsSeq
    int64_t abs_seqno = unwrap(header.seqno + static_cast<int>(header.syn), _isn_no.value(), _check_point);
    // std::cerr << "payload: " << payload.copy() << " stream_idx: " << stream_idx
    // << std::endl;
    // 将任何数据或流结束标记推到StreamReassembler，序号使用Stream Index，即AbsSeq - 1
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - 1, header.fin);
    // std::cerr << "buffer writens: " << stream_out().bytes_written() <<
    // std::endl;
    _check_point += seg.length_in_sequence_space();
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_isn_no == std::nullopt) {  // syn not set
        return std::nullopt;
    }
    // Stream Index -> AbsSeq
    uint64_t written = stream_out().bytes_written() + 1;
    if (stream_out().input_ended()) {
        written++;
    }
    // AbsSeq -> Seq
    return wrap(written, _isn_no.value());
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
