#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>

#define MAX_CLIENT 10
#define MAX_IDLE_WAIT 300
#define SERV_TCP_PORT 52002