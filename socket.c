#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h >
#include < sys/socket.h>
#include < netinet/in.h>
#include <arpa/inet.h>
#include < unistd.h>

static bool stop = false;
