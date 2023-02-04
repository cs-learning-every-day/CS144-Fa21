#### CS144 实验笔记

- [x] Lab 0 Networking Warmup
- [x] Lab 1 Stitching Substrings Into a Byte Stream
- [x] Lab 2 the TCP Receiver
- [x] Lab 3 the TCP Sender
- [ ] Lab 4 the TCP connection
- [ ] Lab 5 the network interface
- [ ] Lab 6 IP routing
- [ ] Lab 7 Putting it all together (Use your own Internet infrastructures to talk with each other, so cool ! )


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

#### Lab4
先通读《Linux高性能服务器编程》Ch1-4 TCP相关的内容，理解TCP状态机。
去tcp_state.cc 看TCP某个状态下sender和receiver的状态。
![Pasted-image-20230203094714](https://cdn.staticaly.com/gh/XmchxUp/cloudimg@master/20230204/Pasted-image-20230203094714.1lcwl1q38lmo.webp)
![image](https://cdn.staticaly.com/gh/XmchxUp/cloudimg@master/20230204/image.69pye5ub0qs0.webp)
![image](https://cdn.staticaly.com/gh/XmchxUp/cloudimg@master/20230204/image.3fg6td8rc140.webp)
![image](https://cdn.staticaly.com/gh/XmchxUp/cloudimg@master/20230204/image.72r1vsp9g0g0.webp)