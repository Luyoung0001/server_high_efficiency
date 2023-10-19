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

#### 发送和接收带外数据

```c
        const char *oob_data = "abc";
        const char *normal_data = "123";
        // 发送数据

        send(sockfd, normal_data, strlen(normal_data), 0); // 默认
        send(sockfd, oob_data, strlen(oob_data), MSG_OOB); // 带外数据
        send(sockfd, normal_data, strlen(normal_data), 0); // 再次发送正常数据
```
```c
        memset(buffer, '\0', BUF_SIZE);
        ret = recv(connfd, buffer, BUF_SIZE - 1, MSG_OOB);
        printf("got %d bytes of oob data '%s'\n", ret, buffer);
```

#### 公网 ip 无法 bind

这是因为阿里云的公网 ip 不是真正的公网ip,那是 EIP,也就是弹性 ip.

EIP 和公网 ip 有很多的差别,一下主要罗列几点.
- EIP支持专用网络，公网ip支持专有网络和经典网络；
- EIP能单独持有，公网ip不可以；
- EIP能在云服务器上弹性插拔，公网ip不能；
- EIP网卡分为可见模式和多EIP网卡可见模式下可见，而公网IP在经典网络可见，专有网络不可见；

以上都是在腾讯云 EIP 官方文档中找的,当然只有好处.事实上,EIP 是有一个服务器来分配的.当用户访问 EIP 的时候,云服务器的服务器会将这个请求映射到内部的云服务器上,也就是说,用户并不直接和云服务器进行交互.但是这个server of servers 对用户是透明的,虽然有时候对程序员不怎么透明.

这也是为什么 bind 的时候,无法绑定到EIP 上,只能绑定到本地网卡分配的 ip上,从而实现流量监听.

#### 多 ip 主机怎么实现的

多 ip 主机很好实现,很多服务器会有多张网卡,每一张网卡都可以绑定一个 ip,这样服务器里面跑的服务就可以监听不同的 ip,这样可以做到网卡负载均衡,提高系统吞吐量等等.

#### 为什么无法绑定阿里云提供的公网 IP

上面已经谈到了,EIP 并没有直接与服务器交互,server of servers 对 来自 EIP 的请求做了转发,转发到了内网机组上,也就是你的云服务器上,因此你只能去绑定本机网卡分配的ip.


## UDP 数据读写

由于 UDP 中没有连接的概念,所以每次读取数据都需要获取发送端的 socket 地址, 即参数 src_addr 所指的内容, addrlen 参数指定该地址的长度.

```c
ssize_t recvfrom(int socktd, void *buf, size_t len, int flags, struct sockaddr* src_addr,socklen_t*addrlen);

ssize_t sendto(int socktd, const void *buf, size_t len, int flags, const struct sockaddr* dest_addr,socklen_t addrlen);
```

这两个函数也可以面向连接(STREAM) 的 socket 的数据读写,只需要把最后两个参数都设置成 NULL 以忽略发送端/接收端的 socket 地址.

## 通用数据读写函数

我们在编写服务的过程中,更多的会使用比较高级的,通用的发送\接收系统调用,它们不仅能用于 TCP,还能用于 UDP.

```c
ssize_t recvmsg(int sockfd, struct msghdr* msg,int flags);

ssize_t sendmsg(int sockfd, struct msghdr* msg,int flags);

```

但是在使用这个系统调用之前,得有一些准备工作,比如初始化 msghdr 结构体:

```c
#include<sys/socket.h>
struct msghdr  {
    void  * msg_name ;
    socklen_t msg_namelen ;
    struct iovec  * msg_iov ;
     int  msg_iovlen ;
    void  * msg_control ;
    socklen_t msg_controllen ;
     int  msg_flags ;
} ;

ssize_t recvmsg ( int  sockfd ,  struct msghdr  * msg ,   int  flags ) ;
ssize_t sendmsg ( int  sockfd ,  struct msghdr  * msg ,   int  flags ) ;

```
msg_name：这是一个指向存储消息的目标地址信息的指针。通常，它指向一个 struct sockaddr 结构，用于指定消息的目标地址。如果不需要指定目标地址，可以将其设置为 NULL。

msg_namelen：这是 msg_name 指针指向的目标地址信息的长度。通常，它被设置为 sizeof(struct sockaddr) 或特定协议的地址结构的大小。

msg_iov：这是一个指向 struct iovec 结构体数组的指针。struct iovec 结构体用于指定消息的数据缓冲区，以便在发送和接收数据时可以传递多个数据块。通常，msg_iov 指向一个或多个 struct iovec 结构，每个结构包含一个数据缓冲区的地址和长度。

msg_iovlen：这是 msg_iov 指向的 struct iovec 结构体数组的长度，表示要传递的数据块的数量。

msg_control：这是一个指向辅助数据（ancillary data）的指针，用于传递与消息关联的其他信息，如控制消息、辅助数据等。通常，它指向一个 struct cmsghdr 结构体或其他特定的数据结构，用于存储附加信息。

msg_controllen：这是 msg_control 指向的辅助数据的长度，表示辅助数据的大小。

msg_flags：这是一个整数，用于指定消息的标志或属性。不同的标志可以控制消息的行为，如 MSG_PEEK（查看数据但不删除）、MSG_OOB（带外数据）、MSG_WAITALL（等待接收完整数据）等.这个参数无需指定,因为它会复制函数中flags的字段以影响读写过程,recvmsg 还会在调用结束前,将某些更新后的标志设置到 msg_flags中.


以上,对于 recvmsg 而言,数据读取并存在 msg_iovlen块分散的内存中,这些内存的位置和长度则由 msg_iov指向的数组指定,这称为分散读.

对于 sendmsg 而言,msg_iovlen块分散内存中的数据将一并发送,这称为集中写(gather write).

## 带外标记

内核能检测到 TCP 紧急标志,并且通知应用程序有外带数据需要接收.

方式是:
- I/O复用产生的异常事件
- SIGURG 信号

但是应用程序得到了带外数据需要接收的通知,还需要知道带外数据在数据流中的具体位置,才能准确接受带外数据.

这一点可以通过以下系统调用来实现:

```c
#include <sys/socket.h>
int sockatmark(int sockfd);
```

这个函数检测这个 sockfd 是否处于带外标记,即下一个读取到的数据是否是带外数据

如果是,返回 1,走的是  flags = MSG_OOB 的接收流程;

如果否,则返回 0,走的是 flags = 0 的接收流程;

## 地址信息函数

某些情况下,我们想要知道一个连接 socket 本端 socket 地址,以及远端 socket 地址.

```c
#include <sys/socket.h>
int getsockname(int sockfd, struct sockaddr *addrname, socklen_t * address_len);

int getpeername(int sockfd, struct sockaddr *addrname, socklen_t * address_len);
```

## socket 选项

fcntl（File Control）是UNIX和Linux操作系统提供的系统调用之一，它用于执行对文件描述符的控制操作。文件描述符可以是文件、套接字、管道等I/O资源。

下面两个方法是专门用来读取和设置socket 文件描述符的方法:

```c
int getsockopt(int sockfd, int level, int option_name, void *option_value, socklen_t *restrict option_len);

int setsockopt(int sockfd, int level, int option_name, const void*option_value,socketlen_t option_len);
```

## SO_REUSEADDR 选项

```c
int sock=socket(PF_INET,SOCK STREAM,0);
assert(sock>=0);
int reuse=1;
setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
struct sockaddr_in address;
bzero( &address,sizeof(address));
address.sin_family=AF_INET;
inet_pton(AF_INET,ip,&address.sin_addr);
address.sin_port=htons(port);
int ret=bind(sock,(struct sockaddr*)&address,sizeof(address));
```

setsockopt 之后,就可以立即重用处于 TIME_WAIT 状态的端口.

## SO_RCVBUF, SO_SNDBUF 选项

这两个字段可以直接设置接收以及发送缓冲区的大小.
















