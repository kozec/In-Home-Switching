#include "network.h"
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>


void networkInit(const SocketInitConfig* conf)
{
    //socketInitializeDefault();
    socketInitialize(conf);
    nxlinkStdio();
    avformat_network_init();
}

void networkDestroy() 
{
    avformat_network_deinit();
    socketExit();
}

void setNetStatus(JoyConSocket* connection, const char* fmt, ...)
{
    va_list args;
    mutexLock(&connection->net_status_mut);
    va_start(args, fmt);
    vsnprintf(connection->net_status, sizeof(connection->net_status), fmt, args);
    va_end (args);
    mutexUnlock(&connection->net_status_mut);
}

void getNetStatus(JoyConSocket* connection, char* target)
{
    mutexLock(&connection->net_status_mut);
    memcpy(target, connection->net_status, sizeof(connection->net_status));
    mutexUnlock(&connection->net_status_mut);
}

JoyConSocket* createJoyConSocket()
{
    JoyConSocket* connection = (JoyConSocket*)malloc(sizeof(JoyConSocket));
    mutexInit(&connection->net_status_mut);
    connection->target_len = 0;
    connection->heldKeys = 0;
    setNetStatus(connection, "Initializing...");
    printf("%s\n", connection->net_status);

    connection->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connection->sock >= 0) {
        struct sockaddr_in si;
        si.sin_family = AF_INET;
        si.sin_port = htons(2223);
        si.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(connection->sock, (struct sockaddr*)&si, sizeof(si)) < 0) {
            connection->sock = -1;
        }
    }
    if (connection->sock >= 0) {
        u32 ip = gethostid();
        if (ip == (127 + 0 + 0 + (1 << 24))) {
            setNetStatus(connection, "Listening on localhost. Is wifi enabled?");
        } else {
            setNetStatus(connection, "Waiting for connection");
        }
    } else {
        setNetStatus(connection, "Failed listen for incomming connections");
    }
    printf("%s\n", connection->net_status);
    return connection;
}

void freeJoyConSocket(JoyConSocket* connection)
{
    free(connection);
}

void sendInput(JoyConSocket* connection, const JoyPkg* pkg)
{
    if (connection->sock < 0) {
        // Failed to created socket
        return;
    }
    fd_set rfds, wfds;
    FD_ZERO(&rfds); FD_ZERO(&wfds);
    FD_SET(connection->sock, &rfds);
    FD_SET(connection->sock, &wfds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10;
    int available = select(connection->sock + 1, &rfds, &wfds, NULL, &timeout);
    if (available < 1) {
        printf("Cannot send joycon data; Output buffer not yet ready\n");
    } else {
       if (FD_ISSET(connection->sock, &rfds)) {
           // Got packet from PC. I don't care what's inside, but from now,
           // I'll send joycon data to its adress
           char recvbuffer[8];
           connection->target_len = sizeof(connection->target);
           int r = recvfrom(connection->sock, recvbuffer, 8, 0, (struct sockaddr*)&connection->target, &connection->target_len);
           if (r < 0) {
               setNetStatus(connection, "recvfrom failed: errno=%i (%s)", errno, strerror(errno));
               printf("%s\n", connection->net_status);
               connection->target_len = 0;
               return;
           } else {
               u8* b = (u8*)&connection->target.sin_addr.s_addr;
               setNetStatus(connection, "Connected to %u.%u.%u.%u", b[0], b[1], b[2], b[3]);
           }
       }
       if (FD_ISSET(connection->sock, &wfds)) {
           if (connection->target_len > 0) {
               // Ready to send data
               int r = sendto(connection->sock, (char*)pkg, 0x14, 0, (struct sockaddr*)&connection->target, connection->target_len);
               u8* b = (u8*)&connection->target.sin_addr.s_addr;
               if (r < 0) {
                   setNetStatus(connection, "sendto %u.%u.%u.%u:%i failed: errno=%i (%s)",
                               b[0], b[1], b[2], b[3],
                               ntohs(connection->target.sin_port),
                               errno, strerror(errno));
                   printf("%s\n", connection->net_status);
               }
           }
       }
    }
}

