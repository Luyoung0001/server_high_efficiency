## Linux 网络编程 API
### socket
- socket 地址 API
- socket 基础 API
- 网络信息 API

### socket 地址 api

#### 主机字节序和网络字节序

大端字节序,小端字节序

主机字节序,网络字节序

甚至同一主机中的两个进程都采用不同的字节序.

#### 通用socket地址
#### 1. 创建 socket
int socket();
#### 2. 命名 socket
int bind();
#### 3. 监听 socket
int listen();

#### 4. 接受连接
int accept();

`socket.c`的测试结果表明:
accept()只是在监听队列中取出连接,不论连接处于何种状态(比如 ESTABLISHED, CLOSE_WAIT),更不关心网络状况(比如断网).

#### 5. 发起连接

客户端一般都需要主动发起连接,才能建立连接.
int connect();

#### 6. 关闭连接
close(int sockfd);

sockfd 是待关闭的socket 文件描述符,close()并不是立即关闭这个连接,而是将 fd 的引用计数减一.只有当 fd 的引用计数为 0 时候,才真正关闭连接.

多进程程序中,一次 fork()会将父进程中打开的 socket的引用数加1(这是因为,父进程和子进程共享这个 fd,并不是所有的 fd 都有共享).

因此我们必须在父进程和子进程都对该 socket 执行close()才能将连接关闭.

这样比较繁琐,我们需要在子线程中将这个该死的 socket 关闭时,可能需要关闭多次.因此我们可以直接用 shutdown()这个系统调用直接将连接断掉.

#### shutdown(int sockfd, int howto)
howto:关闭的行为

#### 数据读写
TCP 数据读写: socket编程接口提供了几个专门用于 socket 数据读写的系统调用,它们增加了对数据读写的控制.

ssize_t recv(int sockfd, void*buf,size_t len, int flags);

ssize_t send(int sockfd, const void *buf, size_t len, int flags);

#### 发送带外数据
