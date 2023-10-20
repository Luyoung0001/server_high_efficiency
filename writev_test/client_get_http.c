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
#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc <= 3) {
        printf("usage:%s ipaddress port_num\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1]; // 拿到 ip
    int port = atoi(argv[2]); // 拿到端口
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET; // 设置地址族
    inet_pton(
        AF_INET, ip,
        &server_address.sin_addr); // 转化成 2 进制,存储在 address.sin_addr
    server_address.sin_port = htons(
        port); // 大端存储,因为端口号需要在网络上传输;地址不需要表示都是一样

    // 创建 socket
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    if (connect(sockfd, (struct sockaddr *)&(server_address),
                sizeof(server_address)) < 0) {
        // 连接失败
        printf("connection failed\n");

    } else {
        // 连接成功,获取数据
        char buffer[BUFF_SIZE];
        memset(buffer, '\0', BUFF_SIZE);
        // 接收数据
        int ret = recv(sockfd, buffer, BUFF_SIZE - 1, 0);
        printf("got %d bytes of normal data '%s'\n", ret, buffer);

    }
    close(sockfd);

    return 0;
}