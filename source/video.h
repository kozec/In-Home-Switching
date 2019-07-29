#ifndef _VIDEO_H
#define _VIDEO_H

#include "context.h"

/* Allocates a video context and all its av fields (frame, rgbaFrame ...) */
VideoContext* createVideoContext(void);

/* Loop to handle video streaming with the server */
int handleVid(VideoContext* context);

/* Deallocates a video context */
void freeVideoContext(VideoContext* context);

/* Stops video and changes listening mode */
void setVideoMode(VideoContext* context, bool udp_mode);

bool getVideoMode(VideoContext* context);

/* Loop for receiving and decoding video */
void videoLoop(void *context_ptr);
#endif
