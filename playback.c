//
//          |                |                   |
//   __ \   |   _` |  |   |  __ \    _` |   __|  |  /      __|
//   |   |  |  (   |  |   |  |   |  (   |  (       <      (
//   .__/  _| \__,_| \__, | _.__/  \__,_| \___| _|\_\ _| \___|
//  _|               ____/
//

#include "playback.h"
#include <assert.h>

/*********************************************************
 * Audio Playback Functions
*********************************************************/

static int audioContextInit(AudioContext *audio_ctx) {
    if (!(audio_ctx->want = SDL_malloc(sizeof(SDL_AudioSpec)))) {
        printf("ERROR: Can't allocate memory for SDL_AudioSpec.\n");
        return 1;
    }
    if (!(audio_ctx->have = SDL_malloc(sizeof(SDL_AudioSpec)))) {
        printf("ERROR: Can't allocate memory for SDL_AudioSpec.\n");
        return 1;
    }
    if (!(audio_ctx->audio_queue = av_malloc(sizeof(PacketQueue)))) {
        printf("ERROR: Can't allocate memory for PacketQueue.\n");
        return 1;
    }
    SDL_zero(*audio_ctx->want);
    SDL_zero(*audio_ctx->have);
    audio_ctx->audio_stream = -1;
    audio_ctx->position = 0;
    audio_ctx->duration = 0;
    audio_ctx->format_ctx = NULL;
    audio_ctx->codec_ctx = NULL;
    audio_ctx->audio_codec = NULL;
    audio_ctx->codec_par = NULL;
    audio_ctx->audio_device = -1;
    audio_ctx->time_base = 1;
    return 0;
}

static void audioContextFree(AudioContext *audio_ctx) {
    if (!audio_ctx) { return; }
    if (audio_ctx->want) {
        SDL_free(audio_ctx->want);
        audio_ctx->want = NULL;
    }
    if (audio_ctx->have) {
        SDL_free(audio_ctx->have);
        audio_ctx->have = NULL;
    }
    if (audio_ctx->codec_ctx) {
        avcodec_free_context(&audio_ctx->codec_ctx);
    }
    if (audio_ctx->format_ctx) {
        avformat_close_input(&audio_ctx->format_ctx);
    }
    packetQueueFree(audio_ctx->audio_queue);
    SDL_CloseAudioDevice(audio_ctx->audio_device);
    av_free(audio_ctx);
}

int playAudio(AppState *state, const char *filename) {
    AVPacket *pPacket;
    AudioContext *audio_ctx = NULL;
    int ret;

    if (!(state->audio_ctx = av_malloc(sizeof(AudioContext)))) {
        printf("ERROR: Can't allocate memory for AudioContext.\n");
        return 1;
    }

    audio_ctx = state->audio_ctx;
    if (audioContextInit(audio_ctx) != 0) {
        printf("ERROR: Can't initialize audio context.\n");
        audioContextFree(audio_ctx);
        state->audio_ctx = NULL;
        return 1;
    }

    audio_ctx->format_ctx = avformat_alloc_context();
    if (avformat_open_input(&audio_ctx->format_ctx, filename, NULL, NULL) != 0) {
        printf("ERROR: Can't read context of file %s.\n", filename);
        return 1;
    }

    if (avformat_find_stream_info(audio_ctx->format_ctx, NULL) != 0) {
        printf("WARNING: Couldn't find stream information.\n");
    }

    for (int i = 0; i < audio_ctx->format_ctx->nb_streams; i++) {
        if (audio_ctx->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_ctx->audio_stream = i;
        }
    }
    if (audio_ctx->audio_stream == -1) {
        printf("ERROR: Couldn't find audio stream of file %s.\n", filename);
        return 1;
    }

    // TIMEBASE = NUM / DEN
    audio_ctx->duration = audioGetDuration(state->audio_ctx->format_ctx);
    audio_ctx->time_base = av_q2d(audio_ctx->format_ctx->streams[audio_ctx->audio_stream]->time_base);
    audio_ctx->codec_par = audio_ctx->format_ctx->streams[audio_ctx->audio_stream]->codecpar;
    if (!(audio_ctx->audio_codec = avcodec_find_decoder(audio_ctx->codec_par->codec_id))) {
        printf("ERROR: Unsupported codec.\n");
        return 1;
    }
    audio_ctx->codec_ctx = avcodec_alloc_context3(audio_ctx->audio_codec);
    if (avcodec_parameters_to_context(audio_ctx->codec_ctx, audio_ctx->codec_par) != 0) {
        printf("ERROR: Couldn't find audio codec context.\n");
        return 1;
    }

    audio_ctx->want->freq = audio_ctx->codec_ctx->sample_rate;
    audio_ctx->want->channels = (uint8_t) audio_ctx->codec_ctx->ch_layout.nb_channels;
    audio_ctx->want->format = AUDIO_S16SYS;
    audio_ctx->want->silence = 0;
    audio_ctx->want->samples = SDL_AUDIO_BUFFER_SIZE;
    audio_ctx->want->callback = audioCallback;
    audio_ctx->want->userdata = state;

    if (!(audio_ctx->audio_device = SDL_OpenAudioDevice(NULL, 0, audio_ctx->want, audio_ctx->have, SDL_AUDIO_ALLOW_FORMAT_CHANGE))) {
        printf("ERROR: Can't open audio device.\n");
        return 1;
    }

    if (avcodec_open2(audio_ctx->codec_ctx, audio_ctx->audio_codec, NULL) < 0) {
        printf("ERROR: Can't open audio codec.\n");
        return 1;
    }

    pPacket = av_packet_alloc();
    packetQueueInit(audio_ctx->audio_queue);
    state->playing = TRUE;
    SDL_PauseAudioDevice(audio_ctx->audio_device, FALSE);

    for (;;) {
        ret = av_read_frame(audio_ctx->format_ctx, pPacket);
        if ((ret == AVERROR_EOF || avio_feof(audio_ctx->format_ctx->pb))) {
            break;
        }
        if (audio_ctx->format_ctx->pb && audio_ctx->format_ctx->pb->error) {
            break;
        }
        if (pPacket->stream_index == audio_ctx->audio_stream) {
            packetQueuePut(state, pPacket);
        } else {
            av_packet_unref(pPacket);
        }
    }

    av_packet_free(&pPacket);
    return 0;
}

void stopAudio(AppState *state) {
    state->playing = FALSE;
    state->paused = FALSE;
    state->finished = FALSE;
    audioContextFree(state->audio_ctx);
    state->audio_ctx = NULL;
}

/*********************************************************
 * Packet Queue Functions
*********************************************************/

static void packetQueueInit(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));

    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        printf("ERROR: SDL_CreateMutex Error: %s.\n", SDL_GetError());
        return;
    }

    q->cond = SDL_CreateCond();
    if (!q->cond) {
        printf("ERROR: SDL_CreateCond Error: %s.\n", SDL_GetError());
        return;
    }
}

static void packetQueueFree(PacketQueue *q) {
    PacketList *tmp;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
    while (q->first_pkt) {
        tmp = q->first_pkt;
        q->first_pkt = tmp->next;
        av_packet_free(&tmp->pkt);
        av_free(tmp);
    }
    if (q->mutex) {
        SDL_DestroyMutex(q->mutex);
    }
    if (q->cond) {
        SDL_DestroyCond(q->cond);
    }
    av_free(q);
}

static int packetQueuePut(AppState *state, AVPacket *pkt) {
    PacketQueue *q = state->audio_ctx->audio_queue;
    PacketList *avPacketList = av_malloc(sizeof(PacketList));

    if (!avPacketList) { return -1; }

    avPacketList->pkt = av_packet_alloc();
    av_packet_move_ref(avPacketList->pkt, pkt);

    avPacketList->next = NULL;
    // position in seconds = PTS * TIMEBASE
    avPacketList->position = (double) avPacketList->pkt->pts * state->audio_ctx->time_base;

    SDL_LockMutex(q->mutex);

    if (!q->last_pkt) {
        q->first_pkt = avPacketList;
        q->current = q->first_pkt;
    } else {
        q->last_pkt->next = avPacketList;
    }

    q->last_pkt = avPacketList;
    q->nb_packets++;
    q->size += avPacketList->pkt->size;

    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);

    return 0;
}

static int packetQueueGet(AppState *state, PacketList **pkt, int block) {
    int ret = -1;
    PacketQueue *q = state->audio_ctx->audio_queue;

    if (!state->playing) { return ret; }

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (!state->playing) { break; }

        if (q->current) {
            *pkt = q->current;
            q->current = (*pkt)->next;
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            if (SDL_CondWaitTimeout(q->cond, q->mutex, PACKET_WAIT_TIME_MS) != 0) {
                state->finished = TRUE;
            }
        }
    }

    SDL_UnlockMutex(q->mutex);
    return ret;
}

void packetQueueSeek(AppState *state, double second) {
    PacketQueue *q = state->audio_ctx->audio_queue;
    int prev_volume;
    if (!state->playing || !q->current) {
        return;
    }
    state->playing = FALSE;
    prev_volume = state->volume;
    state->volume = 0;
    int current_second = (int) q->current->position;
    if (current_second > second) {
        q->current = q->first_pkt;
    }
    while (q->current != q->last_pkt && q->current->position < second) {
        q->current = q->current->next;
    }
    state->playing = TRUE;
    state->volume = prev_volume;
}

/*********************************************************
 * Reading and Decoding Functions
*********************************************************/

static void audioCallback(void *userdata, Uint8 *stream, int len) {
    AppState *state = (AppState *) userdata;
    int len1 = -1;
    int audio_size = -1;
    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    if (!state->playing) { return; }

    while (len > 0) {
        if (!state->playing) { return; }

        if (audio_buf_index >= audio_buf_size) {
            audio_size = audioDecodeFrame(state, audio_buf, sizeof(audio_buf));
            if (audio_size < 0) {
                audio_buf_size = 1024;
                memset(audio_buf, 0, audio_buf_size);
                printf("ERROR: audioDecodeFrame() failed.\n");
            } else {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }

        len1 = audio_buf_size - audio_buf_index;

        if (len1 > len) {
            len1 = len;
        }

        if (state->volume == SDL_MIX_MAXVOLUME)
            memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
        else {
            memset(stream, 0, len1);
            SDL_MixAudioFormat(stream, (uint8_t *) audio_buf + audio_buf_index, AUDIO_S16SYS, len1, state->volume);
        }

        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

static int audioDecodeFrame(AppState *state, uint8_t *audio_buf, int buf_size) {
    AVCodecContext *aCodecCtx = NULL;
    PacketList **pkt_list = malloc(sizeof(PacketList *));
    static int audio_pkt_size = 0;
    int prev_position = 0;

    if (!state->playing) { return -1; }

    aCodecCtx = state->audio_ctx->codec_ctx;
    static AVFrame *avFrame = NULL;
    avFrame = av_frame_alloc();
    if (!avFrame) {
        printf("ERROR: Could not allocate AVFrame.\n");
        return -1;
    }

    int len1 = 0;
    int data_size = 0;

    for (;;) {
        if (!state->playing) { return -1; }

        while (audio_pkt_size > 0) {
            int got_frame = 0;

            int ret = avcodec_receive_frame(aCodecCtx, avFrame);
            if (ret == 0) {
                got_frame = 1;
            }
            if (ret == AVERROR(EAGAIN)) {
                ret = 0;
            }
            if (ret == 0 && state->playing) {
                ret = avcodec_send_packet(aCodecCtx, (*pkt_list)->pkt);
            }
            if (ret == AVERROR(EAGAIN)) {
                ret = 0;
            } else if (ret < 0) {
                printf("ERROR: avcodec_receive_frame error.");
                return -1;
            } else {
                len1 = (*pkt_list)->pkt->size;
            }

            if (len1 < 0) {
                audio_pkt_size = 0;
                break;
            }

            audio_pkt_size -= len1;
            data_size = 0;

            if (got_frame) {
                /*data_size = av_samples_get_buffer_size(
                        NULL,
                        aCodecCtx->ch_layout.nb_channels,
                        avFrame->nb_samples,
                        aCodecCtx->sample_fmt,
                        1
                );
                memcpy(audio_buf, avFrame->data[0], data_size);*/

                // audio resampling
                data_size = audioResample(
                    state,
                    avFrame,
                    AV_SAMPLE_FMT_S16,
                    aCodecCtx->ch_layout.nb_channels,
                    aCodecCtx->sample_rate,
                    audio_buf
                );

                assert(data_size <= buf_size);
            }

            if (data_size <= 0) {
                continue;
            }

            return data_size;
        }

        int ret = packetQueueGet(state, pkt_list, TRUE);
        if (ret < 0 || !(*pkt_list) || !(*pkt_list)->pkt) {
            return -1;
        }

        prev_position = state->audio_ctx->position;
        if (prev_position != (int) (*pkt_list)->position) {
            state->audio_ctx->position = (int) (*pkt_list)->position;
            // Must call it from the main thread
            // g_signal_emit_by_name(state->widgets->positionSld, "update_position", state);
        }
        audio_pkt_size = (*pkt_list)->pkt->size;
    }

    return 0;
}

static int audioResample(
    AppState *state,
    AVFrame *decoded_audio_frame,
    enum AVSampleFormat out_sample_fmt,
    int out_channels,
    int out_sample_rate,
    uint8_t *out_buf
) {
    if (!state->playing) {
        return -1;
    }

    AVCodecContext *audio_decode_ctx = state->audio_ctx->codec_ctx;
    SwrContext *swr_ctx = NULL;
    int ret = 0;
    int64_t in_channel_layout;
    int64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_nb_channels = 0;
    int out_linesize = 0;
    int in_nb_samples = 0;
    int out_nb_samples = 0;
    int max_out_nb_samples = 0;
    uint8_t **resampled_data = NULL;
    int resampled_data_size = 0;

    swr_ctx = swr_alloc();

    if (!swr_ctx) {
        printf("ERROR: swr_alloc error.\n");
        return -1;
    }

    // get input audio channels
    /*in_channel_layout = (audio_decode_ctx->channels ==
                         av_get_channel_layout_nb_channels(audio_decode_ctx->channel_layout)) ?   // 2
                        audio_decode_ctx->channel_layout :
                        av_get_default_channel_layout(audio_decode_ctx->channels);*/
    in_channel_layout = audio_decode_ctx->ch_layout.nb_channels;

    // check input audio channels correctly retrieved
    if (in_channel_layout <= 0) {
        printf("ERROR: in_channel_layout error.\n");
        return -1;
    }

    // set output audio channels based on the input audio channels
    if (out_channels == 1) {
        out_channel_layout = AV_CH_LAYOUT_MONO;
    } else if (out_channels == 2) {
        out_channel_layout = AV_CH_LAYOUT_STEREO;
    } else {
        out_channel_layout = AV_CH_LAYOUT_SURROUND;
    }

    // retrieve number of audio samples (per channel)
    in_nb_samples = decoded_audio_frame->nb_samples;
    if (in_nb_samples <= 0) {
        printf("ERROR: in_nb_samples error.\n");
        return -1;
    }

    // Set SwrContext parameters for resampling
    av_opt_set_int(   // 3
        swr_ctx,
        "in_channel_layout",
        in_channel_layout,
        0
    );

    // Set SwrContext parameters for resampling
    av_opt_set_int(
        swr_ctx,
        "in_sample_rate",
        audio_decode_ctx->sample_rate,
        0
    );

    // Set SwrContext parameters for resampling
    av_opt_set_sample_fmt(
        swr_ctx,
        "in_sample_fmt",
        audio_decode_ctx->sample_fmt,
        0
    );

    // Set SwrContext parameters for resampling
    av_opt_set_int(
        swr_ctx,
        "out_channel_layout",
        out_channel_layout,
        0
    );

    // Set SwrContext parameters for resampling
    av_opt_set_int(
        swr_ctx,
        "out_sample_rate",
        out_sample_rate,
        0
    );

    // Set SwrContext parameters for resampling
    av_opt_set_sample_fmt(
        swr_ctx,
        "out_sample_fmt",
        out_sample_fmt,
        0
    );

    // Once all values have been set for the SwrContext, it must be initialized
    // with swr_init().
    ret = swr_init(swr_ctx);;
    if (ret < 0) {
        printf("ERROR: Failed to initialize the resampling context.\n");
        return -1;
    }

    max_out_nb_samples = out_nb_samples = av_rescale_rnd(
        in_nb_samples,
        out_sample_rate,
        audio_decode_ctx->sample_rate,
        AV_ROUND_UP
    );

    // check rescaling was successful
    if (max_out_nb_samples <= 0) {
        printf("ERROR: av_rescale_rnd error.\n");
        return -1;
    }

    // get number of output audio channels
//    out_nb_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    out_nb_channels = audio_decode_ctx->ch_layout.nb_channels;

    ret = av_samples_alloc_array_and_samples(
        &resampled_data,
        &out_linesize,
        out_nb_channels,
        out_nb_samples,
        out_sample_fmt,
        0
    );

    if (ret < 0) {
        printf("ERROR: av_samples_alloc_array_and_samples() error: Could not allocate destination samples.\n");
        return -1;
    }

    // retrieve output samples number taking into account the progressive delay
    out_nb_samples = av_rescale_rnd(
        swr_get_delay(swr_ctx, audio_decode_ctx->sample_rate) + in_nb_samples,
        out_sample_rate,
        audio_decode_ctx->sample_rate,
        AV_ROUND_UP
    );

    // check output samples number was correctly retrieved
    if (out_nb_samples <= 0) {
        printf("ERROR: av_rescale_rnd error\n");
        return -1;
    }

    if (out_nb_samples > max_out_nb_samples) {
        // free memory block and set pointer to NULL
        av_free(resampled_data[0]);

        // Allocate a samples buffer for out_nb_samples samples
        ret = av_samples_alloc(
            resampled_data,
            &out_linesize,
            out_nb_channels,
            out_nb_samples,
            out_sample_fmt,
            1
        );

        // check samples buffer correctly allocated
        if (ret < 0) {
            printf("ERROR: av_samples_alloc failed.\n");
            return -1;
        }

        max_out_nb_samples = out_nb_samples;
    }

    if (swr_ctx) {
        // do the actual audio data resampling
        ret = swr_convert(
            swr_ctx,
            resampled_data,
            out_nb_samples,
            (const uint8_t **) decoded_audio_frame->data,
            decoded_audio_frame->nb_samples
        );

        // check audio conversion was successful
        if (ret < 0) {
            printf("ERROR: swr_convert_error.\n");
            return -1;
        }

        // Get the required buffer size for the given audio parameters
        resampled_data_size = av_samples_get_buffer_size(
            &out_linesize,
            out_nb_channels,
            ret,
            out_sample_fmt,
            1
        );

        // check audio buffer size
        if (resampled_data_size < 0) {
            printf("ERROR: av_samples_get_buffer_size error.\n");
            return -1;
        }
    } else {
        printf("ERROR: swr_ctx null error.\n");
        return -1;
    }

    // copy the resampled data to the output buffer
    memcpy(out_buf, resampled_data[0], resampled_data_size);

    /*
     * Memory Cleanup.
     */
    if (resampled_data) {
        // free memory block and set pointer to NULL
        av_freep(&resampled_data[0]);
    }

    av_freep(&resampled_data);
    resampled_data = NULL;

    if (swr_ctx) {
        // Free the given SwrContext and set the pointer to NULL
        swr_free(&swr_ctx);
    }

    return resampled_data_size;
}

/*********************************************************
 * Audio ID3V2 and Metadata Retrieving Functions
*********************************************************/

// getAlbumArt function returns AVPacket pointer with attached picture.
// AVPacket should be freed by caller.
AVPacket *getAlbumArt(const char *filename) {
    AVFormatContext *format_ctx = NULL;
    AVPacket *tmp = NULL;
    int i;

    if (avformat_open_input(&format_ctx, filename, NULL, NULL) != 0) {
        printf("avformat_open_input() failed");
        return NULL;
    }

    for (i = 0; i < format_ctx->nb_streams; i++)
        if (format_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            tmp = av_packet_alloc();
            av_packet_move_ref(tmp, &format_ctx->streams[i]->attached_pic);
            break;
        }
    avformat_close_input(&format_ctx);
    return tmp;
}

static guint audioGetDuration(AVFormatContext *format_ctx) {
    if (format_ctx->duration) {
        return (int) format_ctx->duration / 1000000;
    }
    return 0;
}
