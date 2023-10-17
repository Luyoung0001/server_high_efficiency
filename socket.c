#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static bool stop = false;
static void handle_term(int sig) { stop = true; }
int main(int argc, char *argv[]) {
    signal(SIGTERM, handle_term);
    if (argc < 4) {
        printf("usage:%s ipaddress_port_num_backlog\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];    // 拿到 ip
    int port = atoi(argv[2]);    // 拿到端口
    int backlog = atoi(argv[3]); // 拿到 backlog

    // socket(int domain, int type, int protocol);
    // domain 指的是应该使用哪个底层协议族,PF_INET,PF_UNIX,PF_INET6等
    // type 指的是服务类型,对于 TCP/IP, 应该使用 SOCK_STREAM; 对于 UDP,应该使用
    // SOCK_DGRAM protocol
    // 参数是在前两个参数构成的协议集合下,再选择一个具体的协议,几乎所有的情况下,我们这个值都是
    // 0 意思是选择默认的 这样,如果成功创建
    // socket,就会返回一个文件描述符,失败返回-1
    int sock = socket(PF_INET, SOCK_STREAM, 0); // 创造socket
    assert(sock >= 0);

    // 创建一个 ipv4 socket 地址
    //     struct sockaddr_in {
    // 	__uint8_t       sin_len;
    // 	sa_family_t     sin_family;
    // 	in_port_t       sin_port;
    // 	struct  in_addr sin_addr;
    // 	char            sin_zero[8];
    // };
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET; // 设置地址族
    inet_pton(AF_INET, ip,
              &address.sin_addr); // 转化成 2 进制,存储在 address.sin_addr
    address.sin_port = htons(port); // 大端存储,因为端口号需要在网络上传输;地址不需要表示都是一样

    // int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen)
    // bind 将 myaddr 所指的地址分配给未命名的 sockfd 文件描述符,addrlen
    // 参数指出该 socket 地址的长度 bind 成功时候返回 0,失败则返回-1,则设置
    // errno: EACCES :被绑定的地址是受保护的地址,仅超级用户能访问,比如普通用户将
    // socket 绑定到知名服务端口(端口号为 0~1023),bind 将返回这个错误
    // EADDRINUSE: 被绑定的地址正在使用中,比如将 socket 绑定到一个处于 TIME_WAIT
    // 状态的 socket
    int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));

    assert(ret != 1);

    // socket 被命名之后,还不能马上接受客户连接,我们需要利用 listen
    // 系统调用来创建一个监听队列以存放待处理的客户连接 int listen(int sockfd,
    // int backlog); sockfd: 被监听的 socket backlog:
    // 内核监听队列的最大长度,如果这个长度超过 backlog,
    // 服务器将不受理新的客户连接
    ret = listen(sock, backlog);

    assert(ret != 1);

    // 循环等待连接,直到有 SINTERM 信号将它中断

    while (!stop) {
        sleep(1);
    }
    close(sock);

    return 0;
}
