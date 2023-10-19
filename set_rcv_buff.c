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
#define BUFFER_SIZE 1024
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage:%s ipaddress port_num recv_buffer_size\n", argv[0]);
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
    // recv buff size 设置
    int recvbuf = atoi(argv[3]);
    int len = sizefo(recvbuf);

     // 设置缓冲区大小
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf)); // 设置
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf,
               (socklen_t *)&len); // 读取

    printf("the tcp recieve buffer size after setting is %d\n", recvbuf);

    // 绑定
    int ret = bind(sock,(struct sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);

    // 从监听队列拿出 ESTABLISHED 的出来接收连接
    int connfd = accept(sock,(struct sockaddr*)&client,&client_addrlength);

    if(connfd < 0){
        // 连接失败
        printf("err is: %d",errno);
    }else{
        // 连接成功
        char buffer[BUFFER_SIZE];
        memset(buffer,'\0',BUFFER_SIZE);
        while(recv(connfd,buffer,BUFFER_SIZE-1,0)>0){}
        close(connfd);
    }
    close(sock);
    return 0;
}
