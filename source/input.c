#include <switch.h>

#include "context.h"
#include "input.h"
#include "network.h"

void gamePadSend(JoyConSocket* connection)
{
    JoystickPosition lJoy;
    JoystickPosition rJoy;
    JoyPkg pkg;
    
    /* Recieve switch input and generate the package */
    hidScanInput();
    pkg.heldKeys = hidKeysHeld(CONTROLLER_P1_AUTO);
    hidJoystickRead(&lJoy, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
    hidJoystickRead(&rJoy, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);
    pkg.lJoyX = lJoy.dx;
    pkg.lJoyY = lJoy.dy;
    pkg.rJoyX = rJoy.dx;
    pkg.rJoyY = rJoy.dy;

    mutexLock(&connection->net_status_mut);    // Reusing same mutex
    connection->heldKeys = pkg.heldKeys;
    mutexUnlock(&connection->net_status_mut);
    sendJoyConInput(connection, &pkg);
}

unsigned long getHeldKeys(JoyConSocket* connection) {
    mutexLock(&connection->net_status_mut);
    unsigned long heldKeys = connection->heldKeys;
    mutexUnlock(&connection->net_status_mut);
    return heldKeys;
}

void handleInput(JoyConSocket* connection)
{
    gamePadSend(connection);
}

void inputHandlerLoop(void* _connection)
{
    JoyConSocket* connection = (JoyConSocket*)_connection;
    while(appletMainLoop())
    {
        handleInput(connection);
        svcSleepThread(23333333);
    }

    freeJoyConSocket(connection);
}
