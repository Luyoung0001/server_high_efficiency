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
#define BUFFER_SIZE 1024

// 定义两种 HTTP 状态码 和 状态信息
static const char *status_line[2] = {"200 OK", "500 Internal server error"};

int main(int argc, char *argv[]) {
    if (argc <= 3) {
        printf("usage:%s ipaddress port_num file_name\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1]; // 拿到 ip
    int port = atoi(argv[2]); // 拿到端口

    // 设置第三个参数,目标文件
    const char *file_name = argv[3];

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
        // 用于保存 HTTP 应答的状态行, 头部字段和一个空行的缓存区

        char header_buf[BUFFER_SIZE];
        memset(header_buf, '\0', BUFFER_SIZE);

        // 存放目标文件内容的应用程序缓存
        char *file_buf;
        // 用于存放目标文件的属性等
        struct stat file_stat;
        // 记录目标文件是否是有效文件
        bool valid = true;

        // 缓存区 header_buf 目前已经使用了多少字节的空间
        int len = 0;

        if (stat(file_name, &file_stat) < 0) {
            // 如果目标文件不存在
            valid = false;
        } else if (S_ISDIR(file_stat.st_mode)) {
            // 如果目标文件是一个目录
            valid = false;

        } else if (file_stat.st_mode & S_IROTH) {
            // 如果用户有读取文件的权限
            int fd = open(file_name, O_RDONLY);
            file_buf = malloc(file_stat.st_size + 1);
            memset(file_buf, '\0', file_stat.st_size + 1);
            if (read(fd, file_buf, file_stat.st_size) < 0) {
                valid = false;
            }

        } else {

            valid = false;
        }
        // 如果目标文件有效,则发送正常的 HTTP 应答
        if (valid) {
            // 将HTTP 的状态行,"Content_Length"头部字段和一个空行一次加入 header_buf中
            ret = snprintf(header_buf,BUFFER_SIZE-1,"%s%s\r\n","HTTP/1.1",status_line[0]);
            len += ret;
            ret = snprintf(header_buf+len,BUFFER_SIZE-1-len,"Content_Length:%d\r\n",file_stat.st_size);
            len+=ret;
            ret = snprintf(header_buf+len,BUFFER_SIZE-1-len,"%s","\r\n");
            // 利用 writev 将header_buf和 file_buf一块儿写出
            struct iovec iv[2];

            iv[0].iov_base = header_buf;
            iv[0].iov_len = strlen(header_buf);

            iv[1].iov_base = file_buf;
            iv[1].iov_len = file_stat.st_size;

            ret = writev(connfd,iv,2);
        }else{
            // 如果目标文件无效,则通知服务器内部发生了错误
            ret = snprintf(header_buf,BUFFER_SIZE-1,"%s%s\r\n","HTTP/1.1",status_line[1]);
            len+=ret;
            ret = snprintf(header_buf,BUFFER_SIZE-1-len,"%s","\r\n");
            send(connfd,header_buf,strlen(header_buf),0);
        }
        close(connfd);
        free(file_buf);
    }
    close(sock);
    return 0;
}
