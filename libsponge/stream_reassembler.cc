#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
    : _output(capacity), 
    _capacity(capacity),
    // _eof_end_idx(capacity + 1),
    _buf(capacity, 0),
    _bitmap(capacity, false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    auto used_capacity = _unassembled_bytes + _output.buffer_size();
    if (used_capacity >= _capacity) {
        return;
    }

    if (eof) {
        _eof_end_idx = index + data.size() + 1;
    }
    
    // 存入数据
    size_t start_idx = max(index, _first_unassembled_idx);
    // 算出能储存的最大index 例如 容量为2 (0 1)未发出时，不能储存1后面的数据
    size_t end_idx = min(index + data.size(), _first_unassembled_idx + _output.remaining_capacity());
    for (; start_idx < end_idx; start_idx++) {
        auto t = start_idx % _capacity;
        _buf[t] = data[start_idx - index];
        if (!_bitmap[t]) {
            _bitmap[t] = true;
            _unassembled_bytes++;
        }
    }

    // 更新状态 更新第一个未组装的索引
    end_idx = _first_unassembled_idx + _capacity;
    while (_bitmap[_first_unassembled_idx % _capacity] &&
            _first_unassembled_idx < end_idx) {
        _first_unassembled_idx++;
    }

    // 发送数据
    if (_first_unread_idx < _first_unassembled_idx) {
        size_t len = (_first_unassembled_idx - _first_unread_idx);
        string _data(len, 0);
        for (size_t j = 0; j < len; j++) {
            auto t = (j + _first_unread_idx) % _capacity;
            _data[j] = _buf[t];
            _bitmap[t] = false;
        }
        _first_unread_idx = _first_unassembled_idx;
        _unassembled_bytes -= len;
        _output.write(_data);
    }

    // + 1是 判断都为0时的特殊情况(可以用个别的值标记 例如上面构造时用capacity+1初始化)
    // 上面_eof_end_idx记得也要加一
    if (_output.bytes_written() + 1 == _eof_end_idx) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { 
    return _unassembled_bytes; 
}

bool StreamReassembler::empty() const { 
    return unassembled_bytes() == 0; 
}
