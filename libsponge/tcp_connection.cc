#include "tcp_connection.hh"

#include <algorithm>
#include <iostream>
#include <limits>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const {
  return _sender.stream_in().remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const {
  return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const {
  return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const {
  return _time_since_last_segment_received;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
  if (!active()) {
    return;
  }
  // RST
  if (seg.header().rst) {
    unclean_close();
  }
  // LISTEN
  else if (_sender.next_seqno_absolute() == 0 &&
           !_receiver.ackno().has_value()) {
    // 连接有对方启动
    if (seg.header().syn) {
      // SYN_REVD
      _receiver.segment_received(seg);
      connect();
    }
  }
  // SYN_SENT
  else if (_sender.next_seqno_absolute() > 0 &&
           _sender.next_seqno_absolute() == bytes_in_flight() &&
           !_receiver.ackno().has_value()) {
    if (seg.header().syn) {
      if (seg.header().ack) {
        // TCP连接由对方启动，进入ESTABLISHED
        _receiver.segment_received(seg);
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _sender.send_empty_segment();
        send_segments();
      } else {
        // TCP双方同时启动，进入SYN_REVD
        _receiver.segment_received(seg);
        // 主动发送一个ack
        _sender.send_empty_segment();
        send_segments();
      }
    }
  }
  // SYN_RCVD
  else if (_sender.next_seqno_absolute() > 0 &&
           _sender.next_seqno_absolute() == _sender.bytes_in_flight() &&
           _receiver.ackno().has_value() &&
           !_receiver.stream_out().input_ended()) {
    if (seg.header().ack) {
      _receiver.segment_received(seg);
      _sender.ack_received(seg.header().ackno, seg.header().win);
    }
  }
  // ESTABLISHED
  else if (_sender.next_seqno_absolute() > _sender.bytes_in_flight() &&
           !_sender.stream_in().eof()) {
    _receiver.segment_received(seg);
    _sender.ack_received(seg.header().ackno, seg.header().win);
    if (seg.length_in_sequence_space() > 0) {
      _sender.send_empty_segment();
    }
    _sender.fill_window();
    send_segments();
  }
  // FIN_WAIT_1
  else if (_sender.stream_in().eof() &&
           _sender.next_seqno_absolute() ==
               _sender.stream_in().bytes_written() + 2 &&
           _sender.bytes_in_flight() > 0 && _receiver.ackno().has_value() &&
           !_receiver.stream_out().input_ended()) {
    if (seg.header().fin) {
      // -> TIME_WAIT/CLOSING
      _receiver.segment_received(seg);
      _sender.ack_received(seg.header().ackno, seg.header().win);
      _sender.send_empty_segment();
      send_segments();
    } else if (seg.header().ack) {
      // -> FIN_WAIT_2
      _receiver.segment_received(seg);
      _sender.ack_received(seg.header().ackno, seg.header().win);
      send_segments();
    }
  }
  // FIN_WAIT_2
  else if (_sender.stream_in().eof() &&
           _sender.next_seqno_absolute() ==
               _sender.stream_in().bytes_written() + 2 &&
           _sender.bytes_in_flight() == 0 && _receiver.ackno().has_value() &&
           !_receiver.stream_out().input_ended()) {
    if (seg.header().fin) {
      // TIME_WAIT
      _receiver.segment_received(seg);
      _sender.ack_received(seg.header().ackno, seg.header().win);
      _sender.send_empty_segment();
      send_segments();
    }
  }
  // TIME_WAIT
  else if (_sender.stream_in().eof() &&
           _sender.next_seqno_absolute() ==
               _sender.stream_in().bytes_written() + 2 &&
           _sender.bytes_in_flight() == 0 &&
           _receiver.stream_out().input_ended()) {
    if (seg.header().fin) {
      // TIME_WAIT
      _receiver.segment_received(seg);
      _sender.ack_received(seg.header().ackno, seg.header().win);
      _sender.send_empty_segment();
      send_segments();
    }
  }
  // ELSE
  else {
    _receiver.segment_received(seg);
    _sender.ack_received(seg.header().ackno, seg.header().win);
    _sender.fill_window();
    send_segments();
  }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
  size_t res{};
  res = _sender.stream_in().write(data);
  _sender.fill_window();
  send_segments();
  return res;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to
//! this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
  if (!_active) {
    return;
  }
  _time_since_last_segment_received += ms_since_last_tick;
  _sender.tick(ms_since_last_tick);
  if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
    TCPSegment rst{};
    rst.header().rst = true;
    _segments_out.push(rst);
    unclean_close();
  }
  send_segments();
}

void TCPConnection::end_input_stream() {
  // ESTALIBSHED -> FIN_WAIT_1
  _sender.stream_in().end_input();
  _sender.fill_window();
  send_segments();
}

void TCPConnection::connect() {
  // CLOSED -> SYN_SEND
  _sender.fill_window();
  send_segments();
}

void TCPConnection::send_segments() {
  // copy _sender segment out to local
  while (!_sender.segments_out().empty()) {
    TCPSegment seg = _sender.segments_out().front();
    _sender.segments_out().pop();
    // see _sender.segments_out()
    // need set ack and window size from receiver
    if (_receiver.ackno().has_value()) {
      seg.header().ack = true;
      seg.header().ackno = _receiver.ackno().value();
    }
    size_t max_window_size = std::numeric_limits<uint16_t>::max();
    seg.header().win = std::min(max_window_size, _receiver.window_size());
    _segments_out.push(seg);
  }
  clean_close();
}

void TCPConnection::unclean_close() {
  _sender.stream_in().set_error();
  _receiver.stream_out().set_error();
  _active = false;
}

void TCPConnection::clean_close() {
  if (_receiver.stream_out().input_ended()) {
    if (!_sender.stream_in().eof()) {
      _linger_after_streams_finish = false;
    }
    if (_sender.bytes_in_flight() == 0 && _sender.stream_in().eof()) {
      if (!_linger_after_streams_finish ||
          time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
        _active = false;
      }
    }
  }
}

TCPConnection::~TCPConnection() {
  try {
    if (active()) {
      cerr << "Warning: Unclean shutdown of TCPConnection\n";
      // Your code here: need to send a RST segment to the peer
      TCPSegment rst{};
      rst.header().rst = true;
      _segments_out.push(rst);
      unclean_close();
    }
  } catch (const exception &e) {
    std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
  }
}
