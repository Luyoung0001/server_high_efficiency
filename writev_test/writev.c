#include <stdio.h>
#include <sys/uio.h>
#include <strings.h>

int main() {
    static char part2[] = "THIS IS FROM WRITEV";
    static char part3 = 'A';
    static char part1[] = "[";

    struct iovec iov[3];

    iov[0].iov_base = part1;
    iov[0].iov_len = strlen(part1);

    iov[1].iov_base = part2;
    iov[1].iov_len = strlen(part2);

    iov[2].iov_base = &part3;
    iov[2].iov_len = sizeof(char);

    writev(1, iov, 3);

    return 0;
}