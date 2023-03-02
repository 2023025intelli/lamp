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
    int size;
    int nb_packets;
    PacketList *first_pkt;
    PacketList *last_pkt;
    PacketList *current;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

typedef struct AudioContext {
    int position;
    int audio_stream;
    AVCodec *audio_codec;
    AVCodecContext *codec_ctx;
    AVFormatContext *format_ctx;
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
    GtkFileFilter *audioFilter;
    /* Playlist */
    GtkWidget *playlist;
    GtkWidget *playlistLbl;
    GtkWidget *playlistScr;
    GListStore *playlistStore;
    GtkMultiSelection *selection_model;
    /* Icons */
    GtkWidget *repeatImg;
    GtkWidget *shuffleImg;
    GtkWidget *settingsImg;
    GtkWidget *prevImg;
    GtkWidget *stopImg;
    GtkWidget *playImg;
    GtkWidget *pauseImg;
    GtkWidget *nextImg;
    /* Current list item */
    GtkListItem *listItem;
} AppWidgets;

typedef struct AppState {
    gboolean paused;
    gboolean playing;
    gboolean finished;
    int width;
    int height;
    int repeat;
    int volume;
    int art_size;
    int *shuffle_arr;
    int current_index;
    char *current_file;
    int shuffle_current;
    GApplication *app;
    AppWidgets *widgets;
    AudioContext *audio_ctx;
} AppState;

#endif //LAMP4_DATA_TYPES_H
