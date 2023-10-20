#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc <= 3) {
        printf("usage:%s ipaddress port_num file_name\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1]; // 拿到 ip
    int port = atoi(argv[2]); // 拿到端口

    // 设置第三个参数,目标文件
    const char *file_name = argv[3];
    int filefd = open(file_name, O_RDONLY);
    assert(filefd > 0);

    struct stat stat_buf;
    fstat(filefd, &stat_buf); // 获取文件信息

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
    // accept函数由TCP服务器调用，用于从已完成连接队列返回下一个已完成连接。
    // 如果已完成连接队列为空，那么进程被投入睡眠（假定套接字默为默认的阻塞方式）
    int connfd = accept(sock, (struct sockaddr *)&client, &client_addrlength);

    if (connfd < 0) {
        // 连接失败
        printf("errno is: %d\n", errno);
    } else {
        // 连接成功
        sendfile(connfd, filefd, NULL, NULL, NULL, stat_buf.st_size);
        close(connfd);
        close(filefd);
    }
    close(sock);
    return 0;
}
