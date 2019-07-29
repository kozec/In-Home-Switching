#include <switch.h>

#include "context.h"
#include "input.h"
#include "network.h"

#define KEEP_TOUCH_FLAG (0x80 << 16)
#define TOUCH_ID(i) (1 << (16 + i))
#define MAX_TOUCHES 3


void gamePadSend(JoyConSocket* connection)
{
    JoystickPosition lJoy;
    JoystickPosition rJoy;
    JoyPkg pkg;
    
    /* Recieve switch input and generate the package */
    hidScanInput();
    u32 touchCount = hidTouchCount();
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
    pkg.heldKeys &= 0xFF00FFFF;                // Clear byte used by sticks as dpad
    
    if (touchCount > 0)
    {
        struct touchPosition touches[MAX_TOUCHES];
        struct touchPosition pos;
        for (u32 i=0; i<touchCount; i++)
        {
            hidTouchRead(&pos, i);
            if (pos.id < MAX_TOUCHES)
            {
                touches[pos.id] = pos;
                pkg.heldKeys |= TOUCH_ID(pos.id);
            }
        }

        if (pkg.heldKeys & (TOUCH_ID(1) | TOUCH_ID(2)))
        {
            // Touch points 2 and 3 are reported only in every even report
            if (connection->touch_report_flipflop)
            {
                connection->touch_report_flipflop = false;
                pkg.heldKeys |= KEEP_TOUCH_FLAG;
                pkg.heldKeys &= ~(TOUCH_ID(1) | TOUCH_ID(2));
            } else {
                connection->touch_report_flipflop = true;
            }
        }
        
        for (u32 i=0; i<MAX_TOUCHES; i++)
        {
            if (pkg.heldKeys & TOUCH_ID(i))
            {
                // Toch points are sorted in reversed order, so 1st point can be
                // reported always, without colliding with stick values
                pkg.touch[2-i].x = touches[i].px;
                pkg.touch[2-i].y = touches[i].py;
            }
        }
    }
    sendInput(connection, &pkg);
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
