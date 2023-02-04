#include "tcp_sender.hh"

#include <random>

#include "tcp_config.hh"

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before
//! retransmitting the oldest outstanding segment \param[in] fixed_isn the
//! Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout,
                     const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()})),
      _initial_retransmission_timeout{retx_timeout},
      _stream(capacity),
      _rto_timeout{retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const {
  return _next_seqno - _send_base_seqno;
}

void TCPSender::fill_window() {
  if (_fin_flag) return;
  // 首先需要先发送syn
  if (!_syn_flag) {
    TCPSegment seg;
    seg.header().syn = true;
    send_tcp_segment(seg);
    _syn_flag = true;
    return;
  }
  // syn acked之后才传有window_size
  size_t window_size = (_window_size == 0) ? 1 : _window_size;
  size_t window_remain = window_size - bytes_in_flight();
  while (window_remain > 0 && !_stream.buffer_empty()) {
    TCPSegment seg;
    seg.payload() = Buffer(
        _stream.read(std::min(TCPConfig::MAX_PAYLOAD_SIZE, window_remain)));
    window_remain -= seg.length_in_sequence_space();
    if (_stream.input_ended() && window_remain > 0 && !_fin_flag) {
      seg.header().fin = true;
      _fin_flag = true;
    }
    send_tcp_segment(seg);
    window_remain = window_size - bytes_in_flight();
  }
  if (_stream.input_ended() && window_remain > 0 && !_fin_flag) {
    TCPSegment seg;
    seg.header().fin = true;
    send_tcp_segment(seg);
    _fin_flag = true;
  }
}

void TCPSender::send_tcp_segment(TCPSegment &seg) {
  seg.header().seqno = next_seqno();
  _segments_out.push(seg);
  _segments_wait.push(seg);
  _next_seqno += seg.length_in_sequence_space();
  if (!_timer_running) {
    _timer_running = true;
    _ticks = 0;
  }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno,
                             const uint16_t window_size) {
  _window_size = window_size;
  uint64_t abs_ack = unwrap(ackno, _isn, _next_seqno);
  if (abs_ack > _next_seqno) return;
  if (abs_ack >= _send_base_seqno) {
    _send_base_seqno = abs_ack;
  }
  while (!_segments_wait.empty()) {  // 弹出ackno之前的所有segment
    auto &seg = _segments_wait.front();
    if (unwrap(seg.header().seqno, _isn, _next_seqno) +
            seg.length_in_sequence_space() >
        abs_ack) {
      break;
    }
    _segments_wait.pop();
    _rto_timeout = _initial_retransmission_timeout;
    _consecutive_retransmissions = 0;
    _ticks = 0;
  }
  // 如果还有没被确认的segment, 启动定时器
  if (!_segments_wait.empty()) {
    _timer_running = true;
  } else {
    _timer_running = false;
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call
//! to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
  if (!_timer_running) {
    return;
  }
  _ticks += ms_since_last_tick;
  if (_ticks >= _rto_timeout && _segments_wait.size() > 0) {
    auto &seg = _segments_wait.front();
    _segments_out.push(seg);
    _ticks = 0;
    // TEST: When filling window, treat a '0' window size as equal to '1' but
    // don't back off RTO" syn acked之后才传有window_size
    if (_window_size > 0 || seg.header().syn) {
      _rto_timeout = _rto_timeout * 2;
      _consecutive_retransmissions += 1;
    }
  }
}

unsigned int TCPSender::consecutive_retransmissions() const {
  return _consecutive_retransmissions;
}

void TCPSender::send_empty_segment() {
  TCPSegment seg;
  seg.header().seqno = wrap(_next_seqno, _isn);
  _segments_out.push(seg);
}
