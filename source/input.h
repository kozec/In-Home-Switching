#ifndef INPUT_H
#define INPUT_H

/* Loop to handle joycon inputs and send theme to the server */
void inputHandlerLoop(void* connection);

unsigned long getHeldKeys(JoyConSocket* connection);

#endif
