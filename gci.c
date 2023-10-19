#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage:%s ipaddress port_num\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1]; // 拿到 ip
    int port = atoi(argv[2]); // 拿到端口

    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    address.sin_family = AF_INET; // 设置地址族
    inet_pton(AF_INET, ip,
              &address.sin_addr); // 转化成 2 进制,存储在 address.sin_addr
    address.sin_port = htons(
        port); // 大端存储,因为端口号需要在网络上传输;地址不需要表示都是一样

    // 创建 socket
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    // 绑定 socket
    int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);


    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    // 接受连接
    int connfd = accept(sock, (struct sockaddr *)&client, &client_addrlength);

    if (connfd < 0) {
        // 连接失败
        printf("errno is: %d\n", errno);
    } else {
        // 连接成功
        close(STDOUT_FILENO); // 关闭标准输出
        dup(connfd); // 此刻标准输出被重定向到了 socket 通道
        char remote[200];
        memset(remote,'\0',200);
        memset(remote,'a',199);
        printf("connected with ip:%s and port:%d\n",
               inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN),
               ntohs(client.sin_port));
        close(connfd);
    }
    close(sock);

    return 0;
}
