## 两者的关系
关系很简单,数据链路层,网络层以及传输层均是在 linux 内核中实现的,但是要向用户提供服务,必须以 api 的形式.
所以就有了 socket,它就是 这组系统调用的API.

API 必须提供以下两点功能:
- 将 用户数据 从用户缓冲区复制到 TCP/UDP 内核发送缓冲区,交付内核来发送数据;或者从TCP/UDP 内核接收缓冲区 中复制数据到用户缓冲区.
- 应用程序可以通过它们来修改内核中各层协议的某些头部信息或其他数据结构,这就像为用户提供了一双手,把手伸向内核,在底层精确控制通信的行为.
  