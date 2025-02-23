#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <libavformat/avformat.h>
#include <switch.h>
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL_FontCache.h"
#include <arpa/inet.h>

#define RESX 1280
#define RESY 720
#define KEEP_ALIVE_INTERVAL 30


typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *yuv_text;


    SDL_Rect rect;

    Mutex texture_mut;

    u8 YPlane[RESX*RESY];
    u8 UPlane[RESX*RESY/4];
    u8 VPlane[RESX*RESY/4];

    bool frame_avail;
    Mutex frame_avail_mut;
    FC_Font* font;

    bool video_active;
    Mutex video_active_mut;

    int overclock_status;
    bool any_key;

} RenderContext;

typedef struct
{
    AVFormatContext *fmt_ctx;
    AVCodecContext *video_dec_ctx; //, *audio_dec_ctx;
    AVStream *video_stream;        //, *audio_stream = NULL;
    int video_stream_idx;          //, audio_stream_idx = -1;
    AVFrame *rgbframe;
    AVFrame *frame;
    uint8_t *video_dst_data[4];
    int video_dst_linesize[4];
    int video_frame_count;
    RenderContext *renderContext;
    bool udp_mode;
    //static int audio_frame_count = 0;
} VideoContext;

typedef struct
{
    int sock;
    socklen_t target_len;
    char net_status[300];
    Mutex net_status_mut;
    unsigned long heldKeys;
    bool touch_report_flipflop;
    struct sockaddr_in target;
    int keep_alives;
} JoyConSocket;

#endif
