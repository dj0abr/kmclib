#define _LINUX_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "websocketserver.h"
#include "../kmlib/kmtimer.h"

#define TcpDataPort_WebSocket 40129