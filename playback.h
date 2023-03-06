/*
**          |                |                   |        |
**   __ \   |   _` |  |   |  __ \    _` |   __|  |  /     __ \
**   |   |  |  (   |  |   |  |   |  (   |  (       <      | | |
**   .__/  _| \__,_| \__, | _.__/  \__,_| \___| _|\_\ _| _| |_|
**  _|               ____/
*/

#ifndef LAMP4_PLAYBACK_H
#define LAMP4_PLAYBACK_H

#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include "data_types.h"

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
#define PACKET_WAIT_TIME_MS 16

/*********************************************************
 * Audio Playback Functions
*********************************************************/

void stopAudio(AppState *state);

int playAudio(AppState *state, const char *filename);

static int audioContextInit(AudioContext *audio_ctx);

static void audioContextFree(AudioContext *audio_ctx);

/*********************************************************
 * Packet Queue Functions
*********************************************************/

static void packetQueueInit(PacketQueue *q);

static void packetQueueFree(PacketQueue *q);

void packetQueueSeek(AppState *state, double second);

static int packetQueuePut(AppState *state, AVPacket *pkt);

static int packetQueueGet(AppState *state, PacketList **pkt, int block);

/*********************************************************
 * Reading and Decoding Functions
*********************************************************/

static void audioCallback(void *userdata, Uint8 *stream, int len);

static int audioDecodeFrame(AppState *state, uint8_t *audio_buf, int buf_size);

static int audioResample(
    AppState *state,
    AVFrame *decoded_audio_frame,
    enum AVSampleFormat out_sample_fmt,
    int out_channels,
    int out_sample_rate,
    uint8_t *out_buf
);

/*********************************************************
 * Audio ID3V2 and Metadata Retrieving Functions
*********************************************************/

AVPacket *getAlbumArt(const char *filename);

static guint audioGetDuration(AVFormatContext *format_ctx);

#endif //LAMP4_PLAYBACK_H
