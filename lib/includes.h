
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>
#include <stdarg.h>
#include <netdb.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netinet/in.h>
#include <assert.h>
#include <libgen.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <regex.h>
#include <dirent.h>
#include <float.h>
#include <limits.h>

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif /*MIN*/

