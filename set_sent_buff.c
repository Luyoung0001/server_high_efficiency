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
#define BUFFER_SIZE 512

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage:%s ipaddress port_num send_buff_size\n", argv[0]);
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
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int sendbuf = atoi(argv[3]); // sendbuf 的大小
    int len = sizeof(sendbuf);   // sendbuf 的字节数

    // 设置缓冲区大小
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf)); // 设置
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf,
               (socklen_t *)&len); // 读取

    printf("the tcp send buffer size after setting is %d\n", sendbuf);
    // 客户端连接
    if (connect(sock, (struct sockaddr *)&server_address,
                sizeof(server_address)) != -1) {
        // 连接成功
        char buffer[BUFFER_SIZE];
        memset(buffer, 'a', BUFFER_SIZE); // 格式化成 'a'
        send(sock, buffer, BUFFER_SIZE, 0);
    }
    close(sock);

    return 0;
}
