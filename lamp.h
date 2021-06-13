
#ifndef LAMP_LAMP_H
#define LAMP_LAMP_H

#endif //LAMP_LAMP_H

typedef struct {
    mpg123_handle *mh;
    char *buffer;
    size_t buffer_size;
    size_t done;
    int err;
    PaSampleFormat format;
    int channels;
    int encoding;
    long rate;
    off_t frames_c;
    double frame_d;
    int duration;
    mpg123_id3v2 *v2;
} m_context;

static GtkTargetEntry row_entries[] = {
        {"GTK_LIST_BOX_ROW", GTK_TARGET_SAME_APP, 0}
};

static GtkTargetEntry file_entries[] = {
        {"AUDIO_FILE", GTK_TARGET_OTHER_APP, 0}
};

static void playlist_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);

static void window_size_allocate(GtkWidget *window, GdkRectangle *allocation, gpointer user_data);

static void play_callback(GtkWidget *widget, gpointer data);

static void pause_callback(GtkWidget *widget, gpointer data);

static void stop_callback(GtkWidget *widget, gpointer data);

static void next_callback(GtkWidget *widget, gpointer data);

static void prev_callback(GtkWidget *widget, gpointer data);

gboolean window_key_press_callback(GtkWidget *widget, GdkEvent *event, gpointer user_data);

static void add_file_to_playlist(GtkListBox *playlist, const char *filename);

static void volume_changed(GtkRange *range, gpointer user_data);

static void position_changed(GtkRange *range, GtkScrollType scroll, double value, gpointer user_data);

static void *play(void *filename);

static void pause_player();

static void play_th_create();

static void reset_global_vars();

static void cleanup_play_th();

static void play_context_init(m_context *ctx, const char *filename);

static void play_context_free(m_context *ctx);

static void set_current_row_filename(GtkListBoxRow *row);

static void keep_above_callback(GtkToggleButton *togglebutton, gpointer user_data);

static void shuffle_callback(GtkToggleButton *togglebutton, gpointer user_data);

static void add_to_playlist_callback(GtkMenuItem *menuitem, gpointer user_data);

static void open_folder_callback(GtkMenuItem *menuitem, gpointer user_data);

static void save_playlist_callback(GtkMenuItem *menuitem, gpointer user_data);

static void open_playlist_callback(GtkMenuItem *menuitem, gpointer user_data);

static void clear_playlist_callback(GtkMenuItem *menuitem, gpointer user_data);

static void play_next(int reverse, int force);

static void stop_play_th();

static void shuffle_playlist();

static void reset_widgets();

static int get_id3_data(m_context *ctx);

static void set_current_filename_title();

static void set_default_album_art();

static void get_file_album_art();

static void highlight_row(int index);

static void unhighlight_row(int index);

static void drag_playlist_row(GtkWidget *widget, GdkDragContext *context, gpointer user_data);

static void playlist_row_drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *data, guint info, guint time, gpointer user_data);

static void playlist_row_drag_data_received(GtkWidget *widget, GdkDragContext *context, int x, int y, GtkSelectionData *data, guint info, guint time, gpointer user_data);

static void playlist_row_drag_file_received(GtkWidget *widget, GdkDragContext *context, int x, int y, GtkSelectionData *data, guint info, guint time, gpointer user_data);

static void save_playlist(const char *filename);

static void clear_playlist();

static void clear_shuffle();

static int get_totatl_count();

static char *seconds_to_str(int seconds);

static double min(double v_1, double v_2);

static double max(double v_1, double v_2);
