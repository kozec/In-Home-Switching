#ifndef _NETWORK_H
#define _NETWORK_H

#include <switch.h>
#include "context.h"

#define UDP_URL "udp://0.0.0.0:2222"
#define TCP_URL "tcp://0.0.0.0:2222"
//#define TCP_RECV_BUFFER "500000"

/* Data to send to server */
typedef struct __attribute__((packed))
{
    unsigned long heldKeys;
    union {
        struct {
            short lJoyX;
            short lJoyY;
            short rJoyX;
            short rJoyY;
            short pad[2];
        };
        struct {
            short x;
            short y;
        } touch[3];
    };
} JoyPkg;

/* Init nx network and av network */
void networkInit(const SocketInitConfig* conf);

/* Deinitialize nx network and av network*/
void networkDestroy(); 

/* Creates the context for sending joycon inputs */
JoyConSocket* createJoyConSocket();

/* Deallocate from memory the constext used to sent joycon inputs */
void freeJoyConSocket(JoyConSocket* connection);


/* Send joycon input over the network */
void sendInput(JoyConSocket* connection, const JoyPkg* pkg);

/* 
 * Binds, listens and accepts connection with the server
 * If the connection was previously opened reuses it
 */
int connectJoyConSocket(JoyConSocket* connection, int port);

/*
 * Returns status message that can be displayed on screen.
 * Target needs to be 300B or larger.
 */
void getNetStatus(JoyConSocket* connection, char* target);

#endif
