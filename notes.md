#### 2022 CS144 实验笔记

- [x] Lab 0 Networking Warmup
- [x] Lab 1 Stitching Substrings Into a Byte Stream
- [x] Lab 2 the TCP Receiver
- [x] Lab 3 the TCP Sender
- [ ] Lab 4 the TCP connection
- [ ] Lab 5 the network interface
- [ ] Lab 6 IP routing
- [ ] Lab 7 Putting it all together (Use your own Internet infrastructures to talk with each other, so cool ! )


#### 笔记参考
- https://tarplkpqsm.feishu.cn/docx/doxcnpBEN4SG3vA9pVyCoANigBh
- https://www.cnblogs.com/weijunji/p/cs144-study-2.html
- https://www.epis2048.net/2022/cs144-lab3/



#### 配置环境
> 跟着文档走，我这里没有使用虚拟机

```
wget https://web.stanford.edu/class/cs144/vm_howto/setup_dev_env.sh
chmod +x setup_dev_env.sh
./setup_dev_env

sudo apt-get install doxygen clang-format
sudo apt install cmake

mkdir build && cd build
cmake ..
make format
make -j4 && make check_lab0
```

> clang-format --style=google -i xx.cc

- [Formatting Cmake](https://dev.to/10xlearner/formatting-cmake-4dle)

#### 调试
> https://www.inlighting.org/archives/2021-cs144-notes
> https://www.cnblogs.com/kangyupl/p/stanford_cs144_labs.html

#### Lab0
get_URL: 模拟文档中做过手动发HTTP报文，详读文档。

ByteStream类似一个双端队列，一端读，一端写。有些描述不太清楚，例如pop_output、peek_output、read中的参数len超出范围该如何做。我这里直接取可用长度的最小值了来处理了。
> 不要怕乱写，一开始很多函数都不知道是啥意思，直接大体写好，然后靠测试来理解。

#### Lab1
面向测试用例编程，细节有点多，本来眼看只剩一个错误，发现逻辑大体不符，修改后又新出几个错误。就人肉Debug + 日志输出来看。

#### Lab2

#### Lab3
[RFC6298](https://datatracker.ietf.org/doc/rfc6298/?include_text=1)-超时重传机制

不是很懂，里面细节有点多，又是面向测试用例编程，多看测试用例。

对着自顶向下看TCP部分。