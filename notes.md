#### 2022 CS144 实验笔记

- [x] Lab 0 Networking Warmup 2022/4/9
- [x] Lab 1 Stitching Substrings Into a Byte Stream
- [ ] Lab 2 the TCP Receiver
- [ ] Lab 3 the TCP Sender
- [ ] Lab 4 the TCP connection
- [ ] Lab 5 the network interface
- [ ] Lab 6 IP routing
- [ ] Lab 7 Putting it all together (Use your own Internet infrastructures to talk with each other, so cool ! )


#### 配置环境
> 跟着文档走，我这里没有使用虚拟机

#### 调试
> https://www.inlighting.org/archives/2021-cs144-notes

#### Lab0
get_URL: 模拟文档中做过手动发HTTP报文，详读文档。

ByteStream类似一个双端队列，一端读，一端写。有些描述不太清楚，例如pop_output、peek_output、read中的参数len超出范围该如何做。我这里直接取可用长度的最小值了来处理了。
> 不要怕乱写，一开始很多函数都不知道是啥意思，直接大体写好，然后靠测试来理解。

#### Lab1
面向测试用例编程，细节有点多，本来眼看只剩一个错误，发现逻辑大体不符，修改后又新出几个错误。就人肉Debug + 日志输出来看。