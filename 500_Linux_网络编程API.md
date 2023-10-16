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