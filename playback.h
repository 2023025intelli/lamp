#ifndef LAMP4_PLAYBACK_H
#define LAMP4_PLAYBACK_H

#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include "data_types.h"

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
#define PACKET_WAIT_TIME_MS 20

static void packetQueueInit(PacketQueue *q);

static void packetQueueFree(PacketQueue *q);

static int packetQueuePut(AppState *state, AVPacket *pkt);

static int packetQueueGet(AppState *state, PacketList **pkt, int block);

static int audioContextInit(AudioContext *audio_ctx);

static void audioContextFree(AudioContext *audio_ctx);

int playAudio(AppState *state, const char *filename);

void stopAudio(AppState *state);

static void audioCallback(void *userdata, Uint8 *stream, int len);

static int audioDecodeFrame(AppState *state, uint8_t *audio_buf, int buf_size);

int audioGetDuration(AVFormatContext *format_ctx);

void packetQueueSeek(AppState *state, double second);

static int audioResample(
        AppState *state,
        AVFrame *decoded_audio_frame,
        enum AVSampleFormat out_sample_fmt,
        int out_channels,
        int out_sample_rate,
        uint8_t *out_buf
);

AVPacket *getAlbumArt(const char *filename);

#endif //LAMP4_PLAYBACK_H
