#ifndef LAMP4_DATA_TYPES_H
#define LAMP4_DATA_TYPES_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <gtk-4.0/gtk/gtk.h>
#include <SDL_thread.h>
#include <SDL2/SDL.h>

typedef struct PacketList {
    AVPacket *pkt;
    double position;
    struct PacketList *next;
} PacketList;

typedef struct PacketQueue {
    PacketList *first_pkt;
    PacketList *last_pkt;
    PacketList *current;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

typedef struct AudioContext {
    int audio_stream;
    int position;
    AVFormatContext *format_ctx;
    AVCodecContext *codec_ctx;
    AVCodec *audio_codec;
    AVCodecParameters *codec_par;
    SDL_AudioDeviceID audio_device;
    SDL_AudioSpec *want, *have;
    PacketQueue *audio_queue;
    double time_base;
} AudioContext;

typedef struct {
    GtkWidget *win;
    GtkWidget *coverImg;
    GtkWidget *titleLbl;
    GtkWidget *albumLbl;
    GtkWidget *artistLbl;
    GtkWidget *durationLbl;
    GtkWidget *volumeSld;
    GtkWidget *repeatTgl;
    GtkWidget *shuffleTgl;
    GtkWidget *settingsBtn;
    GtkWidget *positionSld;
    GtkWidget *positionLbl;
    GtkWidget *prevBtn;
    GtkWidget *stopBtn;
    GtkWidget *playBtn;
    GtkWidget *pauseBtn;
    GtkWidget *nextBtn;
    GtkWidget *playlistLbl;
    GtkWidget *playlist;
    GListStore *playlistStore;
    GtkFileFilter *audioFilter;
    /* Icons */
    GtkWidget *repeatImg;
    GtkWidget *shuffleImg;
    GtkWidget *settingsImg;
    GtkWidget *prevImg;
    GtkWidget *stopImg;
    GtkWidget *playImg;
    GtkWidget *pauseImg;
    GtkWidget *nextImg;
} AppWidgets;

typedef struct AppState {
    gboolean paused;
    gboolean playing;
    gboolean finished;
    int repeat;
    int volume;
    int *shuffle_arr;
    int shuffle_current;
    int current_index;
    char *current_file;
    AppWidgets *widgets;
    GApplication *app;
    AudioContext *audio_ctx;
} AppState;

#endif //LAMP4_DATA_TYPES_H
